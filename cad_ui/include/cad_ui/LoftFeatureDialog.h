#pragma once

#include <QDialog>
#include <vector>
#include "cad_sketch/Sketch.h"

// 前向声明 (Forward declarations)
class QVBoxLayout;
class QHBoxLayout;
class QListWidget;
class QPushButton;
class QLabel;

namespace cad_ui {

class LoftFeatureDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoftFeatureDialog(QWidget* parent = nullptr);
    ~LoftFeatureDialog() = default;

signals:
    // 当用户完成选择并点击“确定”时，发送此信号
    void operationRequested(const std::vector<cad_sketch::SketchPtr>& profiles);

    // 通知主窗口进入或退出草图选择模式
    void selectionModeChanged(bool enabled, const QString& prompt);

public slots:
    // 主窗口会调用这个槽来传递用户选择的草图
    void onSketchSelected(const cad_sketch::SketchPtr& sketch);

private slots:
    void onAddSectionClicked();
    void onRemoveSectionClicked();
    void onClearSectionsClicked();
    void onOkButtonClicked();

private:
    void setupUI();
    void updateProfileList();

    // UI elements
    QVBoxLayout* m_mainLayout;
    QListWidget* m_profileListWidget;
    QPushButton* m_okButton;

    // Data
    std::vector<cad_sketch::SketchPtr> m_selectedProfiles;
};

} // namespace cad_ui#pragma once
