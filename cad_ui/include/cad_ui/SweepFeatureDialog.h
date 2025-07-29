#pragma once

#include <QDialog>
#include <vector>
#include "cad_sketch/Sketch.h" 

// Forward declarations 
class QVBoxLayout;
class QGridLayout;
class QGroupBox;
class QLabel;
class QPushButton;
class QFrame;
class QHBoxLayout;
class QListWidget;

namespace cad_ui {

    class SweepFeatureDialog : public QDialog {
        Q_OBJECT // <--- Qt对象所必需的宏

    public:
        
        explicit SweepFeatureDialog(QWidget* parent = nullptr);
        ~SweepFeatureDialog() = default;

    signals:
        // 当用户完成选择并点击“确定”时，这个信号会被发送出去
        void operationRequested(const cad_sketch::SketchPtr& profile, const cad_sketch::SketchPtr& path);

        // 这个信号用于通知主窗口进入或退出选择模式
        void selectionModeChanged(bool enabled, const QString& prompt);

    private slots:
        // 按钮的点击事件会连接到这些槽函数
        void onSelectProfileClicked();
        void onSelectPathClicked();
        void onSelectionFinished();
        void onOkButtonClicked();

    public slots:
        // 主窗口会调用这个函数来传递选中的草图
        void onSketchSelected(const cad_sketch::SketchPtr& sketch);

    private:
        void setupUI();
        void updateSelectionDisplay();

        // UI elements
        QVBoxLayout* m_mainLayout;
        QGridLayout* m_selectionLayout;

        // Edge selection group
        QLabel* m_profileLabel;
        QFrame* m_profileFrame;
        QHBoxLayout* m_profileFrameLayout;
        QLabel* m_profileSelectionLabel;
        QPushButton* m_profileSelectButton;

        // Path selection group 
        QLabel* m_pathLabel;
        QFrame* m_pathFrame;
        QHBoxLayout* m_pathFrameLayout;
        QLabel* m_pathSelectionLabel;
        QPushButton* m_pathSelectButton;

        // Control buttons
        QPushButton* m_okButton;
        QPushButton* m_cancelButton;

        cad_sketch::SketchPtr m_profileSketch; 
        cad_sketch::SketchPtr m_pathSketch;    

        // 状态变量，用于区分当前是在选择轮廓还是路径
        enum class SelectionState { None, SelectingProfile, SelectingPath };
        SelectionState m_currentSelectionState;
    };

} // namespace cad_ui#pragma once
