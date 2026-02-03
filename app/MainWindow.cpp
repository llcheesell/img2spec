#include "app/MainWindow.h"
#include "core/SpectrogramBuilder.h"
#include "core/Stft.h"
#include "core/GriffinLim.h"
#include "core/Leveling.h"
#include "core/WavWriter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QScrollArea>
#include <iostream>

namespace img2spec {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , imageLoader_(std::make_unique<ImageLoader>())
{
    setupUI();
    setWindowTitle("img2spec - Image to Spectrogram Audio Generator");
    resize(1000, 800);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    auto* centralWidget = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(centralWidget);

    // === Image Section ===
    auto* imageGroup = new QGroupBox("Image", this);
    auto* imageLayout = new QVBoxLayout(imageGroup);

    openButton_ = new QPushButton("Open Image...", this);
    connect(openButton_, &QPushButton::clicked, this, &MainWindow::onOpenImage);
    imageLayout->addWidget(openButton_);

    // Image preview with scroll area
    auto* scrollArea = new QScrollArea(this);
    imagePreview_ = new QLabel(this);
    imagePreview_->setMinimumSize(400, 300);
    imagePreview_->setAlignment(Qt::AlignCenter);
    imagePreview_->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    imagePreview_->setText("No image loaded");
    scrollArea->setWidget(imagePreview_);
    scrollArea->setWidgetResizable(true);
    imageLayout->addWidget(scrollArea, 1);

    mainLayout->addWidget(imageGroup, 1);

    // === Parameters Section ===
    auto* paramsGroup = new QGroupBox("Parameters", this);
    auto* paramsLayout = new QVBoxLayout(paramsGroup);

    // Row 1: Sample Rate & Bit Depth
    auto* row1Layout = new QHBoxLayout();
    row1Layout->addWidget(new QLabel("Sample Rate:", this));
    sampleRateCombo_ = new QComboBox(this);
    sampleRateCombo_->addItems({"44100", "48000", "96000"});
    sampleRateCombo_->setCurrentIndex(0);
    row1Layout->addWidget(sampleRateCombo_);

    row1Layout->addWidget(new QLabel("Bit Depth:", this));
    bitDepthCombo_ = new QComboBox(this);
    bitDepthCombo_->addItems({"16 bit (PCM)", "24 bit (PCM)", "32 bit (Float)"});
    bitDepthCombo_->setCurrentIndex(0);
    row1Layout->addWidget(bitDepthCombo_);
    row1Layout->addStretch();
    paramsLayout->addLayout(row1Layout);

    // Row 2: FFT Size & Hop Size
    auto* row2Layout = new QHBoxLayout();
    row2Layout->addWidget(new QLabel("FFT Size:", this));
    fftSizeCombo_ = new QComboBox(this);
    fftSizeCombo_->addItems({"1024", "2048", "4096"});
    fftSizeCombo_->setCurrentIndex(1);
    row2Layout->addWidget(fftSizeCombo_);

    row2Layout->addWidget(new QLabel("Hop Size:", this));
    hopSizeCombo_ = new QComboBox(this);
    hopSizeCombo_->addItems({"NFFT/2", "NFFT/4", "NFFT/8"});
    hopSizeCombo_->setCurrentIndex(1);
    row2Layout->addWidget(hopSizeCombo_);
    row2Layout->addStretch();
    paramsLayout->addLayout(row2Layout);

    // Row 3: Frequency Scale
    auto* row3Layout = new QHBoxLayout();
    row3Layout->addWidget(new QLabel("Frequency Scale:", this));
    freqScaleCombo_ = new QComboBox(this);
    freqScaleCombo_->addItems({"Linear", "Logarithmic"});
    freqScaleCombo_->setCurrentIndex(0);
    row3Layout->addWidget(freqScaleCombo_);
    row3Layout->addStretch();
    paramsLayout->addLayout(row3Layout);

    // Row 4: minDb & gamma
    auto* row4Layout = new QHBoxLayout();
    row4Layout->addWidget(new QLabel("Min dB:", this));
    minDbSpin_ = new QDoubleSpinBox(this);
    minDbSpin_->setRange(-120.0, -20.0);
    minDbSpin_->setValue(-80.0);
    minDbSpin_->setSingleStep(5.0);
    row4Layout->addWidget(minDbSpin_);

    row4Layout->addWidget(new QLabel("Gamma:", this));
    gammaSpin_ = new QDoubleSpinBox(this);
    gammaSpin_->setRange(0.2, 4.0);
    gammaSpin_->setValue(1.0);
    gammaSpin_->setSingleStep(0.1);
    row4Layout->addWidget(gammaSpin_);
    row4Layout->addStretch();
    paramsLayout->addLayout(row4Layout);

    // Row 5: Griffin-Lim Iterations
    auto* row5Layout = new QHBoxLayout();
    row5Layout->addWidget(new QLabel("Griffin-Lim Iterations:", this));
    iterationsSpin_ = new QSpinBox(this);
    iterationsSpin_->setRange(16, 256);
    iterationsSpin_->setValue(64);
    iterationsSpin_->setSingleStep(8);
    row5Layout->addWidget(iterationsSpin_);
    row5Layout->addStretch();
    paramsLayout->addLayout(row5Layout);

    // Row 6: Normalize Target & Output Gain
    auto* row6Layout = new QHBoxLayout();
    row6Layout->addWidget(new QLabel("Normalize Target (dBFS):", this));
    normalizeTargetSpin_ = new QDoubleSpinBox(this);
    normalizeTargetSpin_->setRange(-6.0, 0.0);
    normalizeTargetSpin_->setValue(-1.0);
    normalizeTargetSpin_->setSingleStep(0.5);
    row6Layout->addWidget(normalizeTargetSpin_);

    row6Layout->addWidget(new QLabel("Output Gain (dB):", this));
    outputGainSpin_ = new QDoubleSpinBox(this);
    outputGainSpin_->setRange(-24.0, 12.0);
    outputGainSpin_->setValue(0.0);
    outputGainSpin_->setSingleStep(1.0);
    row6Layout->addWidget(outputGainSpin_);
    row6Layout->addStretch();
    paramsLayout->addLayout(row6Layout);

    // Row 7: Checkboxes
    auto* row7Layout = new QHBoxLayout();
    limiterCheck_ = new QCheckBox("Safety Limiter", this);
    limiterCheck_->setChecked(true);
    row7Layout->addWidget(limiterCheck_);

    stereoCheck_ = new QCheckBox("Stereo (L/R duplicate)", this);
    stereoCheck_->setChecked(false);
    row7Layout->addWidget(stereoCheck_);
    row7Layout->addStretch();
    paramsLayout->addLayout(row7Layout);

    mainLayout->addWidget(paramsGroup);

    // === Render Section ===
    auto* renderLayout = new QHBoxLayout();
    renderButton_ = new QPushButton("Render && Export WAV...", this);
    renderButton_->setEnabled(false);
    connect(renderButton_, &QPushButton::clicked, this, &MainWindow::onRender);
    renderLayout->addWidget(renderButton_);

    cancelButton_ = new QPushButton("Cancel", this);
    cancelButton_->setEnabled(false);
    connect(cancelButton_, &QPushButton::clicked, this, &MainWindow::onCancel);
    renderLayout->addWidget(cancelButton_);

    progressBar_ = new QProgressBar(this);
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    renderLayout->addWidget(progressBar_, 1);

    mainLayout->addLayout(renderLayout);

    setCentralWidget(centralWidget);
}

void MainWindow::onOpenImage() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Image",
        "",
        "Image Files (*.png *.jpg *.jpeg);;All Files (*)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    std::cout << "Opening image: " << fileName.toStdString() << std::endl;

    if (!imageLoader_->load(fileName.toStdString())) {
        QMessageBox::critical(this, "Error", "Failed to load image.");
        return;
    }

    currentImagePath_ = fileName;
    updatePreview();
    renderButton_->setEnabled(true);

    std::cout << "Image loaded successfully." << std::endl;
}

void MainWindow::updatePreview() {
    if (!imageLoader_->isLoaded()) {
        return;
    }

    const int width = imageLoader_->getWidth();
    const int height = imageLoader_->getHeight();
    const auto& data = imageLoader_->getGrayscaleData();

    // Create QImage from grayscale data
    QImage image(width, height, QImage::Format_RGB888);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float value = data[y * width + x];
            const int gray = static_cast<int>(value * 255.0f);
            image.setPixel(x, y, qRgb(gray, gray, gray));
        }
    }

    // Scale to fit preview (max 600px)
    const int maxSize = 600;
    QPixmap pixmap = QPixmap::fromImage(image);
    if (width > maxSize || height > maxSize) {
        pixmap = pixmap.scaled(maxSize, maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    imagePreview_->setPixmap(pixmap);
    imagePreview_->adjustSize();

    std::cout << "Preview updated: " << width << "x" << height << std::endl;
}

void MainWindow::onRender() {
    if (!imageLoader_->isLoaded()) {
        QMessageBox::warning(this, "Error", "No image loaded.");
        return;
    }

    // Get save file path
    QString savePath = QFileDialog::getSaveFileName(
        this,
        "Save WAV File",
        "",
        "WAV Files (*.wav);;All Files (*)"
    );

    if (savePath.isEmpty()) {
        return;
    }

    // Ensure .wav extension
    if (!savePath.endsWith(".wav", Qt::CaseInsensitive)) {
        savePath += ".wav";
    }

    std::cout << "\n=== Starting Render ===" << std::endl;
    std::cout << "Output file: " << savePath.toStdString() << std::endl;

    // Disable UI during rendering
    setUIEnabled(false);
    progressBar_->setValue(0);

    try {
        // Get parameters from UI
        const int sampleRate = sampleRateCombo_->currentText().toInt();
        const int fftSize = fftSizeCombo_->currentText().toInt();

        // Parse hop size
        int hopSize = fftSize / 4; // Default
        QString hopText = hopSizeCombo_->currentText();
        if (hopText.contains("/2")) hopSize = fftSize / 2;
        else if (hopText.contains("/4")) hopSize = fftSize / 4;
        else if (hopText.contains("/8")) hopSize = fftSize / 8;

        const bool isLinear = (freqScaleCombo_->currentIndex() == 0);
        const double minDb = minDbSpin_->value();
        const double gamma = gammaSpin_->value();
        const int iterations = iterationsSpin_->value();
        const double normalizeTarget = normalizeTargetSpin_->value();
        const double outputGain = outputGainSpin_->value();
        const bool useLimiter = limiterCheck_->isChecked();
        const bool stereo = stereoCheck_->isChecked();

        // Parse bit depth
        BitDepth bitDepth = BitDepth::Int16;
        const int bitDepthIdx = bitDepthCombo_->currentIndex();
        if (bitDepthIdx == 0) bitDepth = BitDepth::Int16;
        else if (bitDepthIdx == 1) bitDepth = BitDepth::Int24;
        else if (bitDepthIdx == 2) bitDepth = BitDepth::Float32;

        std::cout << "Parameters:" << std::endl;
        std::cout << "  Sample Rate: " << sampleRate << " Hz" << std::endl;
        std::cout << "  FFT Size: " << fftSize << std::endl;
        std::cout << "  Hop Size: " << hopSize << std::endl;
        std::cout << "  Frequency Scale: " << (isLinear ? "Linear" : "Logarithmic") << std::endl;
        std::cout << "  Min dB: " << minDb << std::endl;
        std::cout << "  Gamma: " << gamma << std::endl;
        std::cout << "  Griffin-Lim Iterations: " << iterations << std::endl;
        std::cout << "  Normalize Target: " << normalizeTarget << " dBFS" << std::endl;
        std::cout << "  Output Gain: " << outputGain << " dB" << std::endl;
        std::cout << "  Safety Limiter: " << (useLimiter ? "ON" : "OFF") << std::endl;
        std::cout << "  Stereo: " << (stereo ? "YES" : "NO") << std::endl;

        progressBar_->setValue(10);

        // Step 1: Build magnitude spectrogram from image
        SpectrogramBuilder specBuilder;
        SpectrogramParams specParams;
        specParams.fftSize = fftSize;
        specParams.hopSize = hopSize;
        specParams.freqScale = isLinear ? FrequencyScale::Linear : FrequencyScale::Logarithmic;
        specParams.minDb = minDb;
        specParams.gamma = gamma;

        auto magnitudeSpec = specBuilder.buildMagnitudeSpectrogram(
            imageLoader_->getGrayscaleData(),
            imageLoader_->getWidth(),
            imageLoader_->getHeight(),
            specParams
        );

        progressBar_->setValue(20);

        // Step 2: Griffin-Lim reconstruction
        Stft stft(fftSize, hopSize);
        GriffinLim griffinLim;

        auto progressCallback = [this](int current, int total) {
            const int progress = 20 + (current * 60 / total);
            progressBar_->setValue(progress);
        };

        std::vector<float> audio = griffinLim.reconstruct(
            magnitudeSpec,
            stft,
            iterations,
            progressCallback,
            nullptr // cancelFlag (TODO: STEP 5)
        );

        progressBar_->setValue(80);

        if (audio.empty()) {
            throw std::runtime_error("Griffin-Lim reconstruction failed");
        }

        // Step 3: Post-processing
        std::cout << "\n=== Post-processing ===" << std::endl;

        Leveling::removeDCOffset(audio);
        std::cout << "  DC offset removed" << std::endl;

        Leveling::normalize(audio, normalizeTarget);
        std::cout << "  Normalized to " << normalizeTarget << " dBFS" << std::endl;

        Leveling::applyGain(audio, outputGain);
        std::cout << "  Applied gain: " << outputGain << " dB" << std::endl;

        if (useLimiter) {
            Leveling::applySafetyLimiter(audio);
            std::cout << "  Safety limiter applied" << std::endl;
        }

        progressBar_->setValue(90);

        // Step 4: Convert to stereo if requested
        std::vector<float> finalAudio = audio;
        int channels = 1;
        if (stereo) {
            finalAudio = Leveling::monoToStereo(audio);
            channels = 2;
            std::cout << "  Converted to stereo" << std::endl;
        }

        // Step 5: Write WAV file
        std::cout << "\n=== Writing WAV file ===" << std::endl;
        WavWriter wavWriter;
        bool success = wavWriter.write(
            savePath.toStdString(),
            finalAudio,
            channels,
            sampleRate,
            bitDepth
        );

        progressBar_->setValue(100);

        if (success) {
            std::cout << "\n=== Render Complete ===" << std::endl;
            QMessageBox::information(this, "Success",
                QString("Audio rendered successfully!\n\nFile: %1\nDuration: %2 seconds")
                    .arg(savePath)
                    .arg(audio.size() / static_cast<double>(sampleRate), 0, 'f', 2));
        } else {
            throw std::runtime_error("Failed to write WAV file");
        }

    } catch (const std::exception& e) {
        std::cerr << "Render error: " << e.what() << std::endl;
        QMessageBox::critical(this, "Render Error",
            QString("Failed to render audio:\n%1").arg(e.what()));
    }

    // Re-enable UI
    setUIEnabled(true);
    progressBar_->setValue(0);
}

void MainWindow::onCancel() {
    // TODO: Implement in STEP 5
}

void MainWindow::setUIEnabled(bool enabled) {
    openButton_->setEnabled(enabled);
    renderButton_->setEnabled(enabled && imageLoader_->isLoaded());
    cancelButton_->setEnabled(!enabled);
}

} // namespace img2spec
