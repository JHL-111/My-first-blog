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
        Q_OBJECT // <--- Qt����������ĺ�

    public:
        
        explicit SweepFeatureDialog(QWidget* parent = nullptr);
        ~SweepFeatureDialog() = default;

    signals:
        // ���û����ѡ�񲢵����ȷ����ʱ������źŻᱻ���ͳ�ȥ
        void operationRequested(const cad_sketch::SketchPtr& profile, const cad_sketch::SketchPtr& path);

        // ����ź�����֪ͨ�����ڽ�����˳�ѡ��ģʽ
        void selectionModeChanged(bool enabled, const QString& prompt);

    private slots:
        // ��ť�ĵ���¼������ӵ���Щ�ۺ���
        void onSelectProfileClicked();
        void onSelectPathClicked();
        void onSelectionFinished();
        void onOkButtonClicked();

    public slots:
        // �����ڻ�����������������ѡ�еĲ�ͼ
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

        // ״̬�������������ֵ�ǰ����ѡ����������·��
        enum class SelectionState { None, SelectingProfile, SelectingPath };
        SelectionState m_currentSelectionState;
    };

} // namespace cad_ui#pragma once
