#include "cad_ui/SweepFeatureDialog.h" // �������Ǹոմ�����ͷ�ļ�
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QMessageBox>

// ������Ҫ����UTF-8��������ȷ��ʾ�����ַ�
#pragma execution_character_set("utf-8")

namespace cad_ui {

    // --- ���캯�� ---
    SweepFeatureDialog::SweepFeatureDialog(QWidget* parent)
        : QDialog(parent), m_currentSelectionState(SelectionState::None) {
        // ����˽�з�������������UI�ؼ�
        setupUI();
        // ����Ϊ��ģ̬�Ի��������û���ѡ��ʱ���ܲ���3D��ͼ
        setModal(false);
        // ȷ���Ի����������ϲ���ʾ
        setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    }

    // --- ����UI�ĺ��ĺ��� ---
    void SweepFeatureDialog::setupUI() {
        setWindowTitle("����ɨ������");
        resize(400, 300);

        m_mainLayout = new QVBoxLayout(this);
        m_mainLayout->setContentsMargins(12, 12, 12, 12);
        m_mainLayout->setSpacing(15);

        QGroupBox* selectionGroup = new QGroupBox("ѡ��", this);
        m_selectionLayout = new QGridLayout(selectionGroup);

        // --- ����(Profile)ѡ������ ---
        m_profileLabel = new QLabel("���� (Profile):", this);
        m_profileFrame = new QFrame(this);
        m_profileFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        m_profileFrameLayout = new QHBoxLayout(m_profileFrame);
        m_profileSelectionLabel = new QLabel("δѡ��", this);
        m_profileSelectButton = new QPushButton("ѡ��", this);

        m_profileFrameLayout->addWidget(m_profileSelectionLabel);
        m_profileFrameLayout->addStretch();
        m_profileFrameLayout->addWidget(m_profileSelectButton);

        m_selectionLayout->addWidget(m_profileLabel, 0, 0);
        m_selectionLayout->addWidget(m_profileFrame, 0, 1);

        // --- ·��(Path)ѡ������ ---
        m_pathLabel = new QLabel("·�� (Path):", this);
        m_pathFrame = new QFrame(this);
        m_pathFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        m_pathFrameLayout = new QHBoxLayout(m_pathFrame);
        m_pathSelectionLabel = new QLabel("δѡ��", this);
        m_pathSelectButton = new QPushButton("ѡ��", this);

        m_pathFrameLayout->addWidget(m_pathSelectionLabel);
        m_pathFrameLayout->addStretch();
        m_pathFrameLayout->addWidget(m_pathSelectButton);

        m_selectionLayout->addWidget(m_pathLabel, 1, 0);
        m_selectionLayout->addWidget(m_pathFrame, 1, 1);

        m_mainLayout->addWidget(selectionGroup);
        m_mainLayout->addStretch();

        // --- ȷ����ȡ����ť ---
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        m_okButton = new QPushButton("ȷ��", this);
        m_cancelButton = new QPushButton("ȡ��", this);

        m_okButton->setEnabled(false); // Ĭ�Ͻ��ã�ֱ����������������

        buttonLayout->addStretch();
        buttonLayout->addWidget(m_cancelButton);
        buttonLayout->addWidget(m_okButton);
        m_mainLayout->addLayout(buttonLayout);

        // --- �����źźͲ� ---
        connect(m_profileSelectButton, &QPushButton::clicked, this, &SweepFeatureDialog::onSelectProfileClicked);
        connect(m_pathSelectButton, &QPushButton::clicked, this, &SweepFeatureDialog::onSelectPathClicked);
        connect(m_okButton, &QPushButton::clicked, this, &SweepFeatureDialog::onOkButtonClicked);
        connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    }

    // ---�ۺ���ʵ��---

    void SweepFeatureDialog::onSelectProfileClicked() {
        m_currentSelectionState = SelectionState::SelectingProfile;
        m_profileSelectButton->setText("ѡ����...");
        m_pathSelectButton->setEnabled(false); // ��ֹ�û�ͬʱѡ��·��
        // �����źţ�֪ͨ�����ڽ��롰��ͼѡ��ģʽ��
        emit selectionModeChanged(true, "����3D��ͼ���ĵ�����ѡ��һ����ͼ��Ϊ'����'");
    }

    void SweepFeatureDialog::onSelectPathClicked() {
        m_currentSelectionState = SelectionState::SelectingPath;
        m_pathSelectButton->setText("ѡ����...");
        m_profileSelectButton->setEnabled(false); // ��ֹ�û�ͬʱѡ������
        emit selectionModeChanged(true, "����3D��ͼ���ĵ�����ѡ��һ����ͼ��Ϊ'·��'");
    }

    // �������ڲ���һ����ͼѡ��󣬻�����������
    void SweepFeatureDialog::onSketchSelected(const cad_sketch::SketchPtr& sketch) {
        if (m_currentSelectionState == SelectionState::SelectingProfile) {
            m_profileSketch = sketch;
            onSelectionFinished(); // ���ѡ��
        }
        else if (m_currentSelectionState == SelectionState::SelectingPath) {
            m_pathSketch = sketch;
            onSelectionFinished(); // ���ѡ��
        }
        updateSelectionDisplay();
    }

    void SweepFeatureDialog::onSelectionFinished() {
        m_currentSelectionState = SelectionState::None;

        // �ָ���ť״̬
        m_profileSelectButton->setText("ѡ��");
        m_pathSelectButton->setText("ѡ��");
        m_profileSelectButton->setEnabled(true);
        m_pathSelectButton->setEnabled(true);

        // �˳�ѡ��ģʽ
        emit selectionModeChanged(false, "");
    }

    void SweepFeatureDialog::onOkButtonClicked() {
        if (m_profileSketch && m_pathSketch) {
            // ��������ѡ���������ͼ��������
            emit operationRequested(m_profileSketch, m_pathSketch);
            accept(); // �رնԻ���
        }
        else {
            QMessageBox::warning(this, "����", "��ȷ����ѡ��������·����");
        }
    }

    // --- �������� ---
    void SweepFeatureDialog::updateSelectionDisplay() {
        // ����������ǩ
        if (m_profileSketch) {
            m_profileSelectionLabel->setText(QString::fromStdString(m_profileSketch->GetName()));
        }
        else {
            m_profileSelectionLabel->setText("δѡ��");
        }

        // ����·����ǩ
        if (m_pathSketch) {
            m_pathSelectionLabel->setText(QString::fromStdString(m_pathSketch->GetName()));
        }
        else {
            m_pathSelectionLabel->setText("δѡ��");
        }

        // ���������·������ѡ�������á�ȷ������ť
        if (m_profileSketch && m_pathSketch) {
            m_okButton->setEnabled(true);
        }
        else {
            m_okButton->setEnabled(false);
        }
    }

} // namespace cad_ui