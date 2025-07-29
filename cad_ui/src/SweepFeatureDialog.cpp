#include "cad_ui/SweepFeatureDialog.h" // 包含我们刚刚创建的头文件
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QMessageBox>

// 我们需要设置UTF-8编码来正确显示中文字符
#pragma execution_character_set("utf-8")

namespace cad_ui {

    // --- 构造函数 ---
    SweepFeatureDialog::SweepFeatureDialog(QWidget* parent)
        : QDialog(parent), m_currentSelectionState(SelectionState::None) {
        // 调用私有方法来创建所有UI控件
        setupUI();
        // 设置为非模态对话框，这样用户在选择时还能操作3D视图
        setModal(false);
        // 确保对话框总在最上层显示
        setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    }

    // --- 创建UI的核心函数 ---
    void SweepFeatureDialog::setupUI() {
        setWindowTitle("创建扫掠特征");
        resize(400, 300);

        m_mainLayout = new QVBoxLayout(this);
        m_mainLayout->setContentsMargins(12, 12, 12, 12);
        m_mainLayout->setSpacing(15);

        QGroupBox* selectionGroup = new QGroupBox("选择", this);
        m_selectionLayout = new QGridLayout(selectionGroup);

        // --- 轮廓(Profile)选择区域 ---
        m_profileLabel = new QLabel("轮廓 (Profile):", this);
        m_profileFrame = new QFrame(this);
        m_profileFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        m_profileFrameLayout = new QHBoxLayout(m_profileFrame);
        m_profileSelectionLabel = new QLabel("未选择", this);
        m_profileSelectButton = new QPushButton("选择", this);

        m_profileFrameLayout->addWidget(m_profileSelectionLabel);
        m_profileFrameLayout->addStretch();
        m_profileFrameLayout->addWidget(m_profileSelectButton);

        m_selectionLayout->addWidget(m_profileLabel, 0, 0);
        m_selectionLayout->addWidget(m_profileFrame, 0, 1);

        // --- 路径(Path)选择区域 ---
        m_pathLabel = new QLabel("路径 (Path):", this);
        m_pathFrame = new QFrame(this);
        m_pathFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        m_pathFrameLayout = new QHBoxLayout(m_pathFrame);
        m_pathSelectionLabel = new QLabel("未选择", this);
        m_pathSelectButton = new QPushButton("选择", this);

        m_pathFrameLayout->addWidget(m_pathSelectionLabel);
        m_pathFrameLayout->addStretch();
        m_pathFrameLayout->addWidget(m_pathSelectButton);

        m_selectionLayout->addWidget(m_pathLabel, 1, 0);
        m_selectionLayout->addWidget(m_pathFrame, 1, 1);

        m_mainLayout->addWidget(selectionGroup);
        m_mainLayout->addStretch();

        // --- 确定和取消按钮 ---
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        m_okButton = new QPushButton("确定", this);
        m_cancelButton = new QPushButton("取消", this);

        m_okButton->setEnabled(false); // 默认禁用，直到所有条件都满足

        buttonLayout->addStretch();
        buttonLayout->addWidget(m_cancelButton);
        buttonLayout->addWidget(m_okButton);
        m_mainLayout->addLayout(buttonLayout);

        // --- 连接信号和槽 ---
        connect(m_profileSelectButton, &QPushButton::clicked, this, &SweepFeatureDialog::onSelectProfileClicked);
        connect(m_pathSelectButton, &QPushButton::clicked, this, &SweepFeatureDialog::onSelectPathClicked);
        connect(m_okButton, &QPushButton::clicked, this, &SweepFeatureDialog::onOkButtonClicked);
        connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    }

    // ---槽函数实现---

    void SweepFeatureDialog::onSelectProfileClicked() {
        m_currentSelectionState = SelectionState::SelectingProfile;
        m_profileSelectButton->setText("选择中...");
        m_pathSelectButton->setEnabled(false); // 防止用户同时选择路径
        // 发送信号，通知主窗口进入“草图选择模式”
        emit selectionModeChanged(true, "请在3D视图或文档树中选择一个草图作为'轮廓'");
    }

    void SweepFeatureDialog::onSelectPathClicked() {
        m_currentSelectionState = SelectionState::SelectingPath;
        m_pathSelectButton->setText("选择中...");
        m_profileSelectButton->setEnabled(false); // 防止用户同时选择轮廓
        emit selectionModeChanged(true, "请在3D视图或文档树中选择一个草图作为'路径'");
    }

    // 当主窗口捕获到一个草图选择后，会调用这个函数
    void SweepFeatureDialog::onSketchSelected(const cad_sketch::SketchPtr& sketch) {
        if (m_currentSelectionState == SelectionState::SelectingProfile) {
            m_profileSketch = sketch;
            onSelectionFinished(); // 完成选择
        }
        else if (m_currentSelectionState == SelectionState::SelectingPath) {
            m_pathSketch = sketch;
            onSelectionFinished(); // 完成选择
        }
        updateSelectionDisplay();
    }

    void SweepFeatureDialog::onSelectionFinished() {
        m_currentSelectionState = SelectionState::None;

        // 恢复按钮状态
        m_profileSelectButton->setText("选择");
        m_pathSelectButton->setText("选择");
        m_profileSelectButton->setEnabled(true);
        m_pathSelectButton->setEnabled(true);

        // 退出选择模式
        emit selectionModeChanged(false, "");
    }

    void SweepFeatureDialog::onOkButtonClicked() {
        if (m_profileSketch && m_pathSketch) {
            // 发送最终选择的两个草图给主窗口
            emit operationRequested(m_profileSketch, m_pathSketch);
            accept(); // 关闭对话框
        }
        else {
            QMessageBox::warning(this, "错误", "请确保已选择轮廓和路径。");
        }
    }

    // --- 辅助函数 ---
    void SweepFeatureDialog::updateSelectionDisplay() {
        // 更新轮廓标签
        if (m_profileSketch) {
            m_profileSelectionLabel->setText(QString::fromStdString(m_profileSketch->GetName()));
        }
        else {
            m_profileSelectionLabel->setText("未选择");
        }

        // 更新路径标签
        if (m_pathSketch) {
            m_pathSelectionLabel->setText(QString::fromStdString(m_pathSketch->GetName()));
        }
        else {
            m_pathSelectionLabel->setText("未选择");
        }

        // 如果轮廓和路径都已选择，则启用“确定”按钮
        if (m_profileSketch && m_pathSketch) {
            m_okButton->setEnabled(true);
        }
        else {
            m_okButton->setEnabled(false);
        }
    }

} // namespace cad_ui