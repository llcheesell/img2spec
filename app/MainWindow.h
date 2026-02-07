#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPixmap>
#include <memory>
#include <vector>

#include "core/ImageLoader.h"
#include "app/ImagePreviewWidget.h"
#include <QAudioSink>
#include <QBuffer>
#include <QProgressDialog>
#include <QTimer>

namespace img2spec {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenImage();
    void onRender();
    void onPreview();
    void onCancel();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void setupUI();
    void updatePreview();
    void updateFrequencyGuides();
    void updateDurationEstimate();
    void setUIEnabled(bool enabled);
    void loadImageFile(const QString& path);
    bool generateAudio(std::vector<float>& finalAudio,
                       int& sampleRate,
                       int& channels,
                       QString* errorMessage,
                       QProgressDialog* progressDialog);
    void startPreviewPlayback(const std::vector<float>& audio, int sampleRate, int channels);
    void stopPreviewPlayback();
    void updatePreviewPosition();

    // UI Components
    ImagePreviewWidget* imagePreview_;
    QLabel* playbackHeaderLabel_;
    QPushButton* openButton_;
    QPushButton* renderButton_;
    QPushButton* previewButton_;
    QPushButton* cancelButton_;
    QProgressBar* progressBar_;

    // Parameters
    QComboBox* sampleRateCombo_;
    QComboBox* bitDepthCombo_;
    QComboBox* fftSizeCombo_;
    QComboBox* hopSizeCombo_;
    QComboBox* freqScaleCombo_;
    QDoubleSpinBox* minFreqSpin_;
    QDoubleSpinBox* maxFreqSpin_;
    QDoubleSpinBox* minDbSpin_;
    QDoubleSpinBox* gammaSpin_;
    QSpinBox* iterationsSpin_;
    QDoubleSpinBox* normalizeTargetSpin_;
    QDoubleSpinBox* outputGainSpin_;
    QCheckBox* limiterCheck_;
    QCheckBox* stereoCheck_;
    QCheckBox* useTargetDurationCheck_;
    QDoubleSpinBox* targetDurationSpin_;

    QLabel* durationLabel_;

    // Data
    std::unique_ptr<ImageLoader> imageLoader_;
    QString currentImagePath_;
    QAudioSink* previewSink_;
    QBuffer* previewBuffer_;
    QTimer* previewPositionTimer_;
    double previewDurationSec_ = 0.0;
};

} // namespace img2spec
