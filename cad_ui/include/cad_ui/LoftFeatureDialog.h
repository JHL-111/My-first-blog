#pragma once

#include <QDialog>
#include <vector>
#include "cad_sketch/Sketch.h"

// ǰ������ (Forward declarations)
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
    // ���û����ѡ�񲢵����ȷ����ʱ�����ʹ��ź�
    void operationRequested(const std::vector<cad_sketch::SketchPtr>& profiles);

    // ֪ͨ�����ڽ�����˳���ͼѡ��ģʽ
    void selectionModeChanged(bool enabled, const QString& prompt);

public slots:
    // �����ڻ����������������û�ѡ��Ĳ�ͼ
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
