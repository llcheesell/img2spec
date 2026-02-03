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
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QProgressDialog>
#include <QApplication>
#include <iostream>
#include <cmath>

namespace img2spec {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , imageLoader_(std::make_unique<ImageLoader>())
{
    setupUI();
    setWindowTitle("img2spec - Image to Spectrogram Audio Generator");
    resize(1000, 800);

    // Enable drag and drop
    setAcceptDrops(true);
    std::cout << "Drag and drop enabled" << std::endl;
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

    // Image preview with frequency guides
    imagePreview_ = new ImagePreviewWidget(this);
    imageLayout->addWidget(imagePreview_, 1);

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

    // Row 3: Frequency Scale & Range
    auto* row3Layout = new QHBoxLayout();
    row3Layout->addWidget(new QLabel("Frequency Scale:", this));
    freqScaleCombo_ = new QComboBox(this);
    freqScaleCombo_->addItems({"Linear", "Logarithmic"});
    freqScaleCombo_->setCurrentIndex(0);
    row3Layout->addWidget(freqScaleCombo_);

    row3Layout->addWidget(new QLabel("Min Freq (Hz):", this));
    minFreqSpin_ = new QDoubleSpinBox(this);
    minFreqSpin_->setRange(10.0, 10000.0);
    minFreqSpin_->setValue(20.0);
    minFreqSpin_->setDecimals(0);
    row3Layout->addWidget(minFreqSpin_);

    row3Layout->addWidget(new QLabel("Max Freq (Hz):", this));
    maxFreqSpin_ = new QDoubleSpinBox(this);
    maxFreqSpin_->setRange(1000.0, 48000.0);
    maxFreqSpin_->setValue(20000.0);
    maxFreqSpin_->setDecimals(0);
    row3Layout->addWidget(maxFreqSpin_);

    row3Layout->addStretch();
    paramsLayout->addLayout(row3Layout);

    // Connect to update frequency guides when changed
    connect(freqScaleCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::updateFrequencyGuides);
    connect(minFreqSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::updateFrequencyGuides);
    connect(maxFreqSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::updateFrequencyGuides);

    // Connect parameters that affect duration
    connect(sampleRateCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::updateDurationEstimate);
    connect(fftSizeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::updateDurationEstimate);
    connect(hopSizeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::updateDurationEstimate);

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

    // === Duration Display ===
    durationLabel_ = new QLabel("Estimated Duration: --", this);
    durationLabel_->setStyleSheet("QLabel { font-size: 12pt; font-weight: bold; color: #4a9eff; padding: 8px; }");
    durationLabel_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(durationLabel_);

    // === Render Section ===
    auto* renderLayout = new QHBoxLayout();
    renderButton_ = new QPushButton("Render && Export WAV...", this);
    renderButton_->setEnabled(false);
    connect(renderButton_, &QPushButton::clicked, this, &MainWindow::onRender);
    connect(renderButton_, &QPushButton::clicked, []() {
        std::cout << "Render button clicked!" << std::endl;
        std::cout.flush();
    });
    renderLayout->addWidget(renderButton_);
    std::cout << "Render button created and connected" << std::endl;

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

void MainWindow::loadImageFile(const QString& path) {
    std::cout << "Loading image: " << path.toStdString() << std::endl;

    if (!imageLoader_->load(path.toStdString())) {
        QMessageBox::critical(this, "Error", "Failed to load image.\nPath: " + path);
        return;
    }

    currentImagePath_ = path;
    updatePreview();
    updateFrequencyGuides();
    updateDurationEstimate();
    renderButton_->setEnabled(true);

    std::cout << "Image loaded successfully: " << imageLoader_->getWidth() << "x"
              << imageLoader_->getHeight() << std::endl;
}

void MainWindow::onOpenImage() {
    std::cout << "Opening file dialog..." << std::endl;

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Image",
        "",
        "Image Files (*.png *.jpg *.jpeg);;All Files (*)",
        nullptr,
        QFileDialog::DontUseNativeDialog  // Force Qt dialog for consistency
    );

    if (fileName.isEmpty()) {
        std::cout << "No file selected" << std::endl;
        return;
    }

    loadImageFile(fileName);
}

void MainWindow::updatePreview() {
    if (!imageLoader_->isLoaded()) {
        imagePreview_->clearImage();
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

    QPixmap pixmap = QPixmap::fromImage(image);
    imagePreview_->setPixmap(pixmap);

    std::cout << "Preview updated: " << width << "x" << height << std::endl;
}

void MainWindow::updateFrequencyGuides() {
    if (!imageLoader_->isLoaded()) {
        return;
    }

    const int freqScale = freqScaleCombo_->currentIndex();
    if (freqScale == 0) {
        // Linear scale: don't show guides (not perceptually meaningful)
        imagePreview_->setFrequencyGuides({});
        return;
    }

    // Logarithmic scale: show frequency guides
    const double minFreq = minFreqSpin_->value();
    const double maxFreq = maxFreqSpin_->value();

    std::vector<FrequencyGuide> guides;

    // Common frequency markers
    std::vector<double> markers = {50, 100, 200, 500, 1000, 2000, 5000, 10000, 15000};

    for (double freq : markers) {
        if (freq >= minFreq && freq <= maxFreq) {
            // Calculate normalized position in log scale [0, 1]
            const double logPos = std::log(freq / minFreq) / std::log(maxFreq / minFreq);
            // Invert (0 = top = high freq, 1 = bottom = low freq)
            const double imagePos = 1.0 - logPos;

            QString label;
            if (freq >= 1000) {
                label = QString::number(freq / 1000.0, 'f', 1) + " kHz";
            } else {
                label = QString::number(freq, 'f', 0) + " Hz";
            }

            guides.push_back({imagePos, label});
        }
    }

    imagePreview_->setFrequencyGuides(guides);
    std::cout << "Frequency guides updated: " << guides.size() << " markers" << std::endl;
}

void MainWindow::updateDurationEstimate() {
    if (!imageLoader_->isLoaded()) {
        durationLabel_->setText("Estimated Duration: --");
        return;
    }

    const int imageWidth = imageLoader_->getWidth();
    const int sampleRate = sampleRateCombo_->currentText().toInt();
    const int fftSize = fftSizeCombo_->currentText().toInt();

    // Parse hop size (e.g., "NFFT/4" -> fftSize/4)
    QString hopText = hopSizeCombo_->currentText();
    int hopSize = fftSize / 4; // default
    if (hopText.contains("/2")) hopSize = fftSize / 2;
    else if (hopText.contains("/8")) hopSize = fftSize / 8;

    // Calculate duration
    // numFrames = imageWidth
    // totalSamples = numFrames * hopSize (approximately, ignoring window overlap)
    const double totalSamples = imageWidth * hopSize;
    const double durationSeconds = totalSamples / sampleRate;

    // Format duration as MM:SS or HH:MM:SS
    const int hours = static_cast<int>(durationSeconds / 3600);
    const int minutes = static_cast<int>((durationSeconds - hours * 3600) / 60);
    const double seconds = durationSeconds - hours * 3600 - minutes * 60;

    QString durationText;
    if (hours > 0) {
        durationText = QString("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 5, 'f', 2, QChar('0'));
    } else {
        durationText = QString("%1:%2")
            .arg(minutes)
            .arg(seconds, 5, 'f', 2, QChar('0'));
    }

    durationLabel_->setText(QString("Estimated Duration: %1 (Sample Rate: %2 Hz, Hop: %3)")
        .arg(durationText)
        .arg(sampleRate)
        .arg(hopSize));

    std::cout << "Duration estimate: " << durationText.toStdString()
              << " (" << durationSeconds << " seconds)" << std::endl;
}

void MainWindow::onRender() {
    std::cout << "\n=== onRender() called ===" << std::endl;
    std::cout.flush();

    if (!imageLoader_->isLoaded()) {
        std::cout << "ERROR: No image loaded!" << std::endl;
        QMessageBox::warning(this, "Error", "No image loaded.");
        return;
    }

    std::cout << "Image is loaded. Opening save dialog..." << std::endl;
    std::cout.flush();

    // Get save file path
    QString savePath = QFileDialog::getSaveFileName(
        this,
        "Save WAV File",
        "",
        "WAV Files (*.wav);;All Files (*)",
        nullptr,
        QFileDialog::DontUseNativeDialog  // Force Qt dialog for consistency
    );

    std::cout << "Save dialog returned: " << (savePath.isEmpty() ? "(cancelled)" : savePath.toStdString()) << std::endl;
    std::cout.flush();

    if (savePath.isEmpty()) {
        return;
    }

    // Ensure .wav extension
    if (!savePath.endsWith(".wav", Qt::CaseInsensitive)) {
        savePath += ".wav";
    }

    std::cout << "\n=== Starting Render ===" << std::endl;
    std::cout << "Output file: " << savePath.toStdString() << std::endl;

    // Create progress dialog
    QProgressDialog progressDialog("Rendering audio from image...", "Cancel", 0, 100, this);
    progressDialog.setWindowTitle("Rendering");
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(0);
    progressDialog.setValue(0);
    progressDialog.show();
    QApplication::processEvents();

    // Disable UI during rendering
    setUIEnabled(false);

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

        progressDialog.setValue(5);
        progressDialog.setLabelText("Building spectrogram from image...");
        QApplication::processEvents();

        // Step 1: Build magnitude spectrogram from image
        SpectrogramBuilder specBuilder;
        SpectrogramParams specParams;
        specParams.fftSize = fftSize;
        specParams.hopSize = hopSize;
        specParams.sampleRate = sampleRate;
        specParams.freqScale = isLinear ? FrequencyScale::Linear : FrequencyScale::Logarithmic;
        specParams.minFreqHz = minFreqSpin_->value();
        specParams.maxFreqHz = maxFreqSpin_->value();
        specParams.minDb = minDb;
        specParams.gamma = gamma;

        auto magnitudeSpec = specBuilder.buildMagnitudeSpectrogram(
            imageLoader_->getGrayscaleData(),
            imageLoader_->getWidth(),
            imageLoader_->getHeight(),
            specParams
        );

        progressDialog.setValue(15);
        progressDialog.setLabelText("Reconstructing phase with Griffin-Lim algorithm...");
        QApplication::processEvents();

        // Step 2: Griffin-Lim reconstruction
        Stft stft(fftSize, hopSize);
        GriffinLim griffinLim;

        auto progressCallback = [&progressDialog](int current, int total) {
            const int progress = 15 + (current * 70 / total);
            progressDialog.setValue(progress);
            progressDialog.setLabelText(QString("Griffin-Lim iteration %1 / %2...").arg(current).arg(total));
            QApplication::processEvents();
        };

        std::vector<float> audio = griffinLim.reconstruct(
            magnitudeSpec,
            stft,
            iterations,
            progressCallback,
            nullptr // cancelFlag (TODO: STEP 5)
        );

        progressDialog.setValue(85);
        progressDialog.setLabelText("Post-processing audio...");
        QApplication::processEvents();

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

        progressDialog.setValue(90);
        QApplication::processEvents();

        // Step 4: Convert to stereo if requested
        std::vector<float> finalAudio = audio;
        int channels = 1;
        if (stereo) {
            finalAudio = Leveling::monoToStereo(audio);
            channels = 2;
            std::cout << "  Converted to stereo" << std::endl;
        }

        // Step 5: Write WAV file
        progressDialog.setValue(95);
        progressDialog.setLabelText("Writing WAV file...");
        QApplication::processEvents();

        std::cout << "\n=== Writing WAV file ===" << std::endl;
        WavWriter wavWriter;
        bool success = wavWriter.write(
            savePath.toStdString(),
            finalAudio,
            channels,
            sampleRate,
            bitDepth
        );

        progressDialog.setValue(100);
        QApplication::processEvents();

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

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    std::cout << "Drag enter event detected" << std::endl;

    if (event->mimeData()->hasUrls()) {
        // Check if any of the URLs is an image file
        for (const QUrl& url : event->mimeData()->urls()) {
            QString path = url.toLocalFile();
            if (path.endsWith(".png", Qt::CaseInsensitive) ||
                path.endsWith(".jpg", Qt::CaseInsensitive) ||
                path.endsWith(".jpeg", Qt::CaseInsensitive)) {
                event->acceptProposedAction();
                std::cout << "Accepting drag: " << path.toStdString() << std::endl;
                return;
            }
        }
    }
    event->ignore();
}

void MainWindow::dropEvent(QDropEvent* event) {
    std::cout << "Drop event detected" << std::endl;

    if (event->mimeData()->hasUrls()) {
        for (const QUrl& url : event->mimeData()->urls()) {
            QString path = url.toLocalFile();
            std::cout << "Dropped file: " << path.toStdString() << std::endl;

            if (path.endsWith(".png", Qt::CaseInsensitive) ||
                path.endsWith(".jpg", Qt::CaseInsensitive) ||
                path.endsWith(".jpeg", Qt::CaseInsensitive)) {
                loadImageFile(path);
                event->acceptProposedAction();
                return; // Only load first valid image
            }
        }
    }
    event->ignore();
}

} // namespace img2spec
