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

#include "core/ImageLoader.h"
#include "app/ImagePreviewWidget.h"

namespace img2spec {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenImage();
    void onRender();
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

    // UI Components
    ImagePreviewWidget* imagePreview_;
    QPushButton* openButton_;
    QPushButton* renderButton_;
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

    QLabel* durationLabel_;

    // Data
    std::unique_ptr<ImageLoader> imageLoader_;
    QString currentImagePath_;
};

} // namespace img2spec
