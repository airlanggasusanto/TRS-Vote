#pragma once
#include <QPushButton>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QLayout>
#include <QWidget>
#include <QLabel>
#include <QMessageBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QStackedWidget>
#include <QListWidget>
#include <QComboBox>
#include <QVector>
#include <QSpinBox>
#include <QGridLayout>
#include <QDAteTime>
#include <QDateTimeEdit>
#include <QDebug>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QProgressBar>
#include <QTabWidget>
#include "votetree.h"

class dialogAbout : public QDialog{
public:
    QVBoxLayout* mainLayout;
    QHBoxLayout* descLayout;
    QLabel* titleLabel;
    QLabel* descLabel;
    dialogAbout() {
        QIcon ico(":/TRSVote/styles/image/dialogico.png");
        QFont fontt("Segoe UI", 14, 600, false);
        QFont descf("Segoe UI", 10, 500, false);
        titleLabel = new QLabel("About TRS Vote");
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setFont(fontt);
        QString desc = "The TRS Vote is an electoral system that utilizes blockchain distributed ledger technology to store votes. It incorporates traceable ring signatures to enhance privacy and security. It is important to note that this system is designed solely for educational purposes as an undergraduate thesis at Universitas Pembangunan Nasional Jawa Timur in 2023.";
        descLabel = new QLabel(desc);
        descLabel->setFont(descf);
        descLabel->setAlignment(Qt::AlignLeft);
        descLabel->setWordWrap(true);
        descLabel->adjustSize();
        mainLayout = new QVBoxLayout;
        mainLayout->setAlignment(Qt::AlignTop);
        mainLayout->addSpacing(15);
        mainLayout->addWidget(titleLabel);
        descLayout = new QHBoxLayout;
        descLayout->setAlignment(Qt::AlignTop);
        descLayout->addWidget(descLabel);
        mainLayout->addSpacing(25);
        mainLayout->addLayout(descLayout);
        setLayout(mainLayout);
        setStyleSheet("background-color:#FFFFFF;");
        setFixedHeight(180);
        setFixedWidth(450);
        setWindowIcon(ico);
    }
    ~dialogAbout() {
        delete titleLabel;
        delete descLabel;
        delete mainLayout;
    }
};

class CustomButton : public QPushButton
{
public:
    explicit CustomButton(QPixmap&normal, QWidget* parent = nullptr)
        : QPushButton(parent)
    {
        btn_size = QSize(50, 50);
        p_normal = normal;
        setIcon(p_normal);
        setIconSize(btn_size);
        setStyleSheet("QPushButton { border: none; }""QPushButton:focus { outline: none; }");
    }

    void resize_btn(const QSize& size) {
        btn_size = size;
        setIconSize(btn_size);
    }

    QPixmap p_normal;
    QPixmap p_hover;
    QPixmap p_clicked;
    QPixmap p_disabled;
    QSize btn_size;

protected:
    void enterEvent(QEnterEvent* event) override {
        setIcon(p_hover);
        QPushButton::enterEvent(event);
    }

    void leaveEvent(QEvent* event) override {
        setIcon(p_normal);
        QPushButton::leaveEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) override {
        setIcon(p_clicked);
        QPushButton::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        setIcon(p_hover);
        QPushButton::mouseReleaseEvent(event);
    }

    void changeEvent(QEvent* event) override {
        QPushButton::changeEvent(event);
        if (event->type() == QEvent::EnabledChange) {
            if (!isEnabled())
                setIcon(p_disabled);
            else
                setIcon(p_normal);
        }
    }
};

class CheckBoxLineEdit : public QGridLayout
{
    Q_OBJECT

public:
    QLineEdit* lineEdit;
    QPushButton* iconButton;
    QIcon iconHide;
    QIcon iconShow;
    CheckBoxLineEdit(QWidget* parent = nullptr) : QGridLayout(parent)
    {

        iconShow = QIcon(":/TRSVote/styles/image/eyeshowbtn.png");
        iconHide = QIcon(":/TRSVote/styles/image/eyehidehoverbtn.png");
        iconButton = new QPushButton();
        iconButton->setIcon(iconShow);
        iconButton->setCheckable(true);
        iconButton->setChecked(false);
        lineEdit = new QLineEdit();
        addWidget(lineEdit, 0, 0);
        addWidget(iconButton, 0, 1);
        QSize btnsize(25, 25);
        iconButton->setFixedSize(btnsize);
        lineEdit->setEchoMode(QLineEdit::Password);
        connect(iconButton, &QPushButton::toggled, this, &CheckBoxLineEdit::togglePasswordVisibility);
        iconButton->setStyleSheet("background-color:#F5F8FA;");
    }

    QString text() {
        return lineEdit->text();
    }

    void setText(QString t) {
        lineEdit->setText(t);
    }

    void clear() {
        lineEdit->clear();
    }

private slots:
    void togglePasswordVisibility(bool checked)
    {
        if (checked) {
            lineEdit->setEchoMode(QLineEdit::Normal); // Show password
            iconButton->setIcon(iconHide);
        }
        else {
            lineEdit->setEchoMode(QLineEdit::Password); // Mask password
            iconButton->setIcon(iconShow);
        }
    }
};

class MenuButton : public QPushButton {
public:
    explicit MenuButton(const QPixmap& normalPixmap, const QPixmap& hoverPixmap, const QPixmap& disabledPixmap, const QPixmap& pressedPixmap, QWidget* parent = nullptr)
        : QPushButton(parent),
        m_normalPixmap(normalPixmap),
        m_hoverPixmap(hoverPixmap),
        m_disabledPixmap(disabledPixmap),
        m_pressedPixmap(pressedPixmap)
    {
        setIcon(normalPixmap);
        QSize bsize = QSize(200, 200);
        setIconSize(bsize);
        // Remove button borders using style sheet
        setStyleSheet("QPushButton { border: none; }""QPushButton:focus { outline: none; }");
    }

protected:
    void enterEvent(QEnterEvent* event) override {
        setIcon(m_hoverPixmap);
        QPushButton::enterEvent(event);
    }

    void leaveEvent(QEvent* event) override {
        setIcon(m_normalPixmap);
        QPushButton::leaveEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) override {
        setIcon(m_pressedPixmap);
        QPushButton::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        setIcon(m_hoverPixmap);
        QPushButton::mouseReleaseEvent(event);
    }

    void changeEvent(QEvent* event) override {
        if (event->type() == QEvent::EnabledChange) {
            if (!isEnabled())
                setIcon(m_disabledPixmap);
            else
                setIcon(m_normalPixmap);
        }
        QPushButton::changeEvent(event);
    }

private:
    QPixmap m_normalPixmap;
    QPixmap m_hoverPixmap;
    QPixmap m_disabledPixmap;
    QPixmap m_pressedPixmap;
};

class MenuButtonLayout : public QHBoxLayout {
public:
    MenuButton* candidateBtn;
    MenuButton* partisipanBtn;
    MenuButton* validatorBtn;
    MenuButton* adminBtn;
    MenuButtonLayout(QWidget* parent = nullptr) : QHBoxLayout(parent) {
        QPixmap adminImgNormal(":/TRSVote/styles/image/admin_normal.png");
        QPixmap adminImgHover(":/TRSVote/styles/image/admin_hover.png");
        QPixmap adminImgClicked(":/TRSVote/styles/image/admin_clicked.png");
        QPixmap adminImgDisabled(":/TRSVote/styles/image/admin_disable.png");

        QPixmap candidateImgNormal(":/TRSVote/styles/image/candidatebutton_normal.png");
        QPixmap partisipanImgNormal(":/TRSVote/styles/image/partisipan_normal.png");
        QPixmap validatorImgNormal(":/TRSVote/styles/image/validator_normal.png");

        QPixmap candidateImgClicked(":/TRSVote/styles/image/candidatebutton_clicked.png");
        QPixmap candidateImgHover(":/TRSVote/styles/image/candidatebutton_hover.png");
        QPixmap candidateImgDisabled(":/TRSVote/styles/image/candidatebutton_disable.png");

        QPixmap partisipanImgClicked(":/TRSVote/styles/image/partisipan_clicked.png");
        QPixmap partisipanImgHover(":/TRSVote/styles/image/partisipan_hover.png");
        QPixmap partisipanImgDisabled(":/TRSVote/styles/image/partisipan_disable.png");

        QPixmap validatorImgClicked(":/TRSVote/styles/image/validator_clicked.png");
        QPixmap validatorImgHover(":/TRSVote/styles/image/validator_hover.png");
        QPixmap validatorImgDisabled(":/TRSVote/styles/image/validator_disable.png");

        adminBtn = new MenuButton(adminImgNormal, adminImgHover, adminImgHover, adminImgClicked);
        candidateBtn = new MenuButton(candidateImgNormal, candidateImgHover, candidateImgDisabled, candidateImgClicked);
        partisipanBtn = new MenuButton(partisipanImgNormal, partisipanImgHover, partisipanImgDisabled, partisipanImgClicked);
        validatorBtn = new MenuButton(validatorImgNormal, validatorImgHover, validatorImgDisabled, validatorImgClicked);
        adminBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        candidateBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        partisipanBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        validatorBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        label1 = new QLabel("Election Adminstrator");
        label2 = new QLabel("Candidate");
        label3 = new QLabel("Voter");
        label4 = new QLabel("Validator");
        label4->hide();
        layout1 = new QVBoxLayout;
        layout2 = new QVBoxLayout;
        layout3 = new QVBoxLayout;
        layout4 = new QVBoxLayout;
        QFont font("Segoe UI", 14, 600, false);
        label1->setFont(font);
        label2->setFont(font);
        label3->setFont(font);
        label4->setFont(font);
        label1->setAlignment(Qt::AlignHCenter);
        label2->setAlignment(Qt::AlignHCenter);
        label3->setAlignment(Qt::AlignHCenter);
        label4->setAlignment(Qt::AlignHCenter);

        layout1->addWidget(adminBtn);
        layout1->addWidget(label1);
        layout1->setSpacing(20);

        layout2->addWidget(candidateBtn);
        layout2->addWidget(label2);
        layout2->setSpacing(20);

        layout3->addWidget(partisipanBtn);
        layout3->addWidget(label3);
        layout3->setSpacing(20);

        layout4->addWidget(validatorBtn);
        layout4->addWidget(label4);
        layout4->setSpacing(20);

        setSpacing(10);
        setAlignment(Qt::AlignHCenter);

        addItem(layout1);
        addItem(layout2);
        addItem(layout3);
        addItem(layout4);
    }
    ~MenuButtonLayout() {
        delete label1;
        delete label2;
        delete label3;
        delete label4;
        delete adminBtn;
        delete candidateBtn;
        delete partisipanBtn;
        delete validatorBtn;
    }
private:
    QLabel* label1;
    QLabel* label2;
    QLabel* label3;
    QLabel* label4;
    QVBoxLayout* layout1;
    QVBoxLayout* layout2;
    QVBoxLayout* layout3;
    QVBoxLayout* layout4;
};

class MenuTitleLayout : public QHBoxLayout {
public:
    MenuTitleLayout(QWidget* parent = nullptr) : QHBoxLayout(parent) {
        label0 = new QLabel("MAIN MENU");
        QFont font("Segoe UI", 32, 600, false);
        label0->setFont(font);
        label0->setAlignment(Qt::AlignHCenter);
        addWidget(label0);
    }

    ~MenuTitleLayout() {
        delete label0;
    }

private:
    QLabel* label0;
};

class LoginForm : public QWidget {
public:
    QLabel* usernameLabel;
    QLineEdit* usernameLineEdit;
    QLabel* passwordLabel;
    CheckBoxLineEdit* passwordLineEdit;
    QPushButton* loginButton;
    QLabel* titleLabel;
    QVBoxLayout* loginLayout;
    QHBoxLayout* loginMainLayout;
    QVBoxLayout* mainLayout;
    QVBoxLayout* pleaseWaitLayout;
    QHBoxLayout* mainPleaseWaitLayout;
    QLabel* pleaseWaitLabel;
    QFrame* loginFrame;
    LoginForm(QWidget* parent = nullptr)
        : QWidget(parent) {
        pleaseWaitLabel = new QLabel("Please Wait...");
        pleaseWaitLabel->setAlignment(Qt::AlignCenter);
        pleaseWaitLayout = new QVBoxLayout();
        mainPleaseWaitLayout = new QHBoxLayout;
        mainPleaseWaitLayout->addStretch(1);
        mainPleaseWaitLayout->addLayout(pleaseWaitLayout);
        mainPleaseWaitLayout->addStretch(1);
        pleaseWaitLayout->addWidget(pleaseWaitLabel);
        pleaseWaitLayout->setAlignment(Qt::AlignCenter);
        passwordLineEdit = new CheckBoxLineEdit;
        usernameLabel = new QLabel("Username:");
        usernameLineEdit = new QLineEdit;
        passwordLabel = new QLabel("Password:");
        QFont font("Segoe UI", 12, 600, false);
        loginButton = new QPushButton("Login");
        titleLabel = new QLabel("Login Page");
        QFont titlef = QFont("Segoe UI", 32, 600);
        loginFrame = new QFrame;
        loginFrame->setFrameShape(QFrame::Box);
        loginFrame->setFrameShadow(QFrame::Sunken);
        loginFrame->setLineWidth(2);
        titleLabel->setAlignment(Qt::AlignCenter);
        loginLayout = new QVBoxLayout();
        loginLayout->addWidget(usernameLabel);
        loginLayout->addWidget(usernameLineEdit);
        loginLayout->addWidget(passwordLabel);
        loginLayout->addLayout(passwordLineEdit);
        loginLayout->setAlignment(Qt::AlignCenter);
        loginFrame->setLayout(loginLayout);
        loginMainLayout = new QHBoxLayout;
        loginMainLayout->addStretch();
        loginMainLayout->addWidget(loginFrame);
        loginMainLayout->addStretch();
        loginMainLayout->setAlignment(Qt::AlignCenter);

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->setAlignment(Qt::AlignLeft);
        buttonLayout->addStretch();
        buttonLayout->setContentsMargins(2, 2, 2, 2);
        buttonLayout->addWidget(loginButton);
        loginLayout->addLayout(buttonLayout);
        mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(titleLabel);
        mainLayout->addSpacing(20);
        mainLayout->addLayout(loginMainLayout);
        mainLayout->addLayout(mainPleaseWaitLayout);
        mainLayout->setAlignment(Qt::AlignCenter);
        setLayout(mainLayout);
        setStyleSheet("QPushButton {"
            "   background-color: #1DA1F2;"
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}" 
            "QPushButton:hover {"
            "   background-color: #0D95E8;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0B7BC0;"
            "}"
            "QLineEdit {"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "}"
            "QPushButton:focus { outline: none; }");
        setFont(font);
        titleLabel->setFont(titlef);
        endWait();
    }

    ~LoginForm() {
        delete usernameLabel;
        delete usernameLineEdit;
        delete passwordLabel;
        delete passwordLineEdit;
        delete loginButton;
        delete titleLabel;
        delete loginLayout;
        delete mainLayout;
        delete loginFrame;
    }

    void makeWait() {
        pleaseWaitLabel->show();
        setDisabled(true);
    }

    void endWait() {
        pleaseWaitLabel->hide();
        setDisabled(false);
    }
};

class ClickableLayoutWidget : public QWidget {
    Q_OBJECT

public:
    QHBoxLayout* buttonLayout;
    QHBoxLayout* mainLayout;
    QHBoxLayout* labelLayout;
    QLabel* pollLabel;
    QString hashvotepoll;
    QPushButton* registerButton;
    QPushButton* voteButton;
    QPushButton* resultButton;
    QPushButton* mailButton;
    QPushButton* labelButton;
    long long startdate;
    long long enddate;
    long long blocktime;
    long long currentcandidate;
    long long currentvoter;
    long long maxcandidate;
    long long maxvoter;
    int ringsize;
    QString pk;
    QString sig;
    bool registeredFlag;
    bool voteFlag;
    bool requestFlag;
    bool nonVFlag;
    bool asCandidateFlag;
    QLabel* flagLabel;
    VoteTree::display display;
    explicit ClickableLayoutWidget(QWidget* parent = nullptr) : QWidget(parent) {
        ringsize = 0;
        currentvoter = 0;
        blocktime = 0;
        currentcandidate = 0;
        maxcandidate = 0;
        maxvoter = 0;
        asCandidateFlag = false;
        flagLabel = new QLabel;
        flagLabel->setAlignment(Qt::AlignVCenter);
        registeredFlag = false;
        voteFlag = false;
        requestFlag = false;
        nonVFlag = true;
        startdate = 0;
        enddate = 0;
        setCursor(Qt::PointingHandCursor);  // Set the cursor to indicate clickability
        setMouseTracking(true);  // Enable mouse tracking to track hover events
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);  // Set the size policy
        // Initialize the hover state and click state to false
        labelButton = new QPushButton;
        buttonLayout = new QHBoxLayout;
        mainLayout = new QHBoxLayout;
        labelLayout = new QHBoxLayout;
        pollLabel = new QLabel("Polling : ");
        QFont font("Segoe UI", 11, 600);
        pollLabel->setFont(font);
        pollLabel->setStyleSheet("margin: 0; background-color: none;");
        registerButton = new QPushButton("Register");
        voteButton = new QPushButton("Vote");
        resultButton = new QPushButton("Result");
        mailButton = new QPushButton("Mail");
        labelLayout->addWidget(labelButton);
        labelButton->setFixedHeight(30);
        labelButton->setFixedWidth(50);
        labelButton->setDisabled(true);
        labelButton->setStyleSheet("QPushButton:disabled{background-color: #FFFFFF; border: none;}");
        labelLayout->addWidget(pollLabel);
        buttonLayout->addWidget(registerButton, 0, Qt::AlignRight);
        registerButton->hide();
        buttonLayout->addWidget(voteButton, 0, Qt::AlignRight);
        voteButton->hide();
        buttonLayout->addWidget(resultButton, 0, Qt::AlignRight);
        buttonLayout->addWidget(flagLabel, 0, Qt::AlignRight);
        buttonLayout->addWidget(mailButton, 0, Qt::AlignRight);
        resultButton->hide();
        buttonLayout->setAlignment(Qt::AlignRight);
        mainLayout->setContentsMargins(7,7, 15, 7);
        mainLayout->addLayout(labelLayout);
        mainLayout->addStretch();
        mainLayout->addLayout(buttonLayout);
        setLayout(mainLayout);
        QString buttonStyle = "QPushButton {"
            "   background-color: #1DA1F2;"
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #0D95E8;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0B7BC0;"
            "}"
            "QPushButton:focus { outline: none; }";
        registerButton->setStyleSheet(buttonStyle);
        voteButton->setStyleSheet(buttonStyle);
        resultButton->setStyleSheet(buttonStyle);
        mailButton->setStyleSheet(buttonStyle);
        labelButton->setStyleSheet("QPushButton {"
            "   background-color: #000000;"
            "   color: #FFFFFF;"
            "   border:none;"
            "outline: none;"
            "}");

        connect(registerButton, &QPushButton::clicked, [&]() {
            emit registerHVP(hashvotepoll);
            });
        connect(voteButton, &QPushButton::clicked, [&]() {
            emit voteHVP(hashvotepoll);
            });
        connect(resultButton, &QPushButton::clicked, [&]() {
            emit resultHVP(hashvotepoll);
            });
        connect(mailButton, &QPushButton::clicked, [&]() {
            emit mailHVP(hashvotepoll);
            });
        flagLabel->setStyleSheet("background-color:none;");
        flagLabel->hide();
    }

    void setLabelColor(int role) {
        long long date = QDateTime::currentSecsSinceEpoch();
        if (date < startdate) {
            labelButton->setStyleSheet("QPushButton:disabled{background-color: #FFFF00; border:none;}");
            resultButton->setVisible(false);
            flagLabel->hide();
            if (role == 1) {
                mailButton->setVisible(true);
                voteButton->setVisible(false);
                registerButton->setVisible(false);
                return;
            }
            else if (role == 2) {
                mailButton->setVisible(false);
                voteButton->setVisible(false);
                if (requestFlag) {
                    flagLabel->setText("Requested");
                    registerButton->setVisible(false);
                    flagLabel->show();
                }
                if (registeredFlag) {
                    flagLabel->setText("Registered");
                    registerButton->setVisible(false);
                    flagLabel->show();
                }
                if (!requestFlag && !registeredFlag) {
                    registerButton->setVisible(true);
                    flagLabel->hide();
                }
            }
            if (currentvoter >= maxvoter && currentcandidate >= maxcandidate) {
                mailButton->setVisible(false);
                registerButton->setVisible(false);
                flagLabel->setText("Closed Registration");
                flagLabel->show();
            }
        }
        else if (date > startdate && date < enddate) {
            labelButton->setStyleSheet("QPushButton:disabled{background-color: #00FF00; border:none;}");
            flagLabel->hide();
            resultButton->setVisible(false);
            mailButton->setVisible(false);
            if (role == 1) {
                flagLabel->setText("Voting in progress");
                flagLabel->show();
                voteButton->setVisible(false);
                registerButton->setVisible(false);
                return;
            }
            else if (role == 2) {
                registerButton->setVisible(false);
            }
            if (nonVFlag) {
                voteButton->setVisible(false);
                flagLabel->setText("Not Registered");
                flagLabel->show();
            }
            if (voteFlag) {
                voteButton->setVisible(false);
                flagLabel->setText("Already Voted");
                flagLabel->show();
            }
            if (asCandidateFlag) {
                voteButton->setVisible(false);
                flagLabel->setText("Registered As Candidate");
                flagLabel->show();
            }
            if (!nonVFlag && !voteFlag && !asCandidateFlag) {
                flagLabel->hide();
                voteButton->show();
            }
        }
        else if (date > enddate) {
            resultButton->setVisible(true);
            mailButton->setVisible(false);
            flagLabel->hide();
            voteButton->setVisible(false);
            registerButton->setVisible(false);
            labelButton->setStyleSheet("QPushButton:disabled{background-color: #FF0000; border:none;}");
        }
    }

    void logoutUser() {
        registerButton->setDisabled(false);
        voteButton->setDisabled(false);
        registeredFlag = false;
        requestFlag = false;
        voteFlag = false;
        nonVFlag = true;
        asCandidateFlag = false;
    }

signals:
    void registerHVP(QString);
    void voteHVP(QString);
    void resultHVP(QString);
    void mailHVP(QString);
};

class LogoutButton : public QVBoxLayout {
    Q_OBJECT

public:
    CustomButton* logoutButton;
    QLabel* logoutLabel;
    QSize logoutButtonSize;
    QSize hoveredsize;
    QFont size;
    QFont sizeH;

    LogoutButton(QWidget* parent = nullptr) : QVBoxLayout(parent) {
        QPixmap logoutnormal(":/TRSVote/styles/image/logoutnormal.png");
        logoutButton = new CustomButton(logoutnormal);
        logoutButton->p_hover.load(":/TRSVote/styles/image/logoutnormal.png");
        logoutButton->p_clicked.load(":/TRSVote/styles/image/logoutnormal.png");
        logoutButton->p_disabled.load(":/TRSVote/styles/image/logoutnormal.png");
        logoutButtonSize = QSize(100, 50);
        hoveredsize = QSize(120, 60);
        logoutButton->resize_btn(logoutButtonSize);
        logoutLabel = new QLabel("Logout");
        size = QFont("Segoe UI", 12, 600);
        sizeH = QFont("Segoe UI", 12, 800);
        logoutLabel->setFont(size);
        logoutLabel->setAlignment(Qt::AlignCenter);
        addWidget(logoutButton);
        addWidget(logoutLabel);
        setAlignment(Qt::AlignBottom);

        logoutButton->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (obj == logoutButton) {
            if (event->type() == QEvent::Enter) {
                onButtonHovered();
                logoutButton->resize_btn(hoveredsize);
                logoutLabel->setFont(sizeH);
            }
            else if (event->type() == QEvent::Leave) {
                onButtonLeft();
                logoutButton->resize_btn(logoutButtonSize);
                logoutLabel->setFont(size);
            }
        }

        return false;
    }

private slots:
    void onButtonHovered() {
        logoutLabel->setStyleSheet("color:#1DA1F2;");
    }

    void onButtonLeft() {
        logoutLabel->setStyleSheet("color:#000000;");
    }
};

class ProfileButton : public QVBoxLayout {
    Q_OBJECT

public:
    CustomButton* logoutButton;
    QLabel* logoutLabel;
    QSize logoutButtonSize;
    QSize hoveredsize;
    QFont size;
    QFont sizeH;

    ProfileButton(QWidget* parent = nullptr) : QVBoxLayout(parent) {
        QPixmap logoutnormal(":/TRSVote/styles/image/pesannormal.png");
        logoutButton = new CustomButton(logoutnormal);
        logoutButton->p_hover.load(":/TRSVote/styles/image/pesannormal.png");
        logoutButton->p_clicked.load(":/TRSVote/styles/image/pesannormal.png");
        logoutButton->p_disabled.load(":/TRSVote/styles/image/pesannormal.png");
        logoutButtonSize = QSize(100, 50);
        logoutButton->resize_btn(logoutButtonSize);
        logoutLabel = new QLabel("Mail");
        size = QFont("Segoe UI", 12, 600);
        sizeH = QFont("Segoe UI", 12, 800);
        logoutLabel->setFont(size);
        logoutLabel->setAlignment(Qt::AlignCenter);
        addWidget(logoutButton);
        addWidget(logoutLabel);
        setAlignment(Qt::AlignBottom);
        hoveredsize = QSize(120, 60);
        logoutButton->installEventFilter(this);
    }
    void hideButton() {
        logoutButton->hide();
        logoutLabel->hide();
    }

    void showButton() {
        logoutButton->show();
        logoutLabel->show();
    }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (obj == logoutButton) {
            if (event->type() == QEvent::Enter) {
                onButtonHovered();
                logoutButton->resize_btn(hoveredsize);
                logoutLabel->setFont(sizeH);
            }
            else if (event->type() == QEvent::Leave) {
                onButtonLeft();
                logoutButton->resize_btn(logoutButtonSize);
                logoutLabel->setFont(size);
            }
        }

        return false;
    }

private slots:
    void onButtonHovered() {
        logoutLabel->setStyleSheet("color:#1DA1F2;");
    }

    void onButtonLeft() {
        logoutLabel->setStyleSheet("color:#000000;");
    }
};

class KeypairButton : public QVBoxLayout {
    Q_OBJECT

public:
    CustomButton* logoutButton;
    QLabel* logoutLabel;
    QSize logoutButtonSize;
    QSize hoveredsize;
    QFont size;
    QFont sizeH;

    KeypairButton(QWidget* parent = nullptr) : QVBoxLayout(parent) {
        QPixmap logoutnormal(":/TRSVote/styles/image/keynormal.png");
        logoutButton = new CustomButton(logoutnormal);
        logoutButton->p_hover.load(":/TRSVote/styles/image/keynormal.png");
        logoutButton->p_clicked.load(":/TRSVote/styles/image/keynormal.png");
        logoutButton->p_disabled.load(":/TRSVote/styles/image/keynormal.png");
        size = QFont("Segoe UI", 12, 600);
        sizeH = QFont("Segoe UI", 12, 800);
        logoutButtonSize = QSize(100, 50);
        hoveredsize = QSize(120, 60);
        logoutButton->resize_btn(logoutButtonSize);
        logoutLabel = new QLabel("Keypair");
        logoutLabel->setFont(size);
        logoutLabel->setAlignment(Qt::AlignCenter);
        addWidget(logoutButton);
        addWidget(logoutLabel);
        setAlignment(Qt::AlignBottom);

        logoutButton->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (obj == logoutButton) {
            if (event->type() == QEvent::Enter) {
                onButtonHovered();
                logoutButton->resize_btn(hoveredsize);
                logoutLabel->setFont(sizeH);
            }
            else if (event->type() == QEvent::Leave) {
                onButtonLeft();
                logoutButton->resize_btn(logoutButtonSize);
                logoutLabel->setFont(size);
            }
        }

        return false;
    }

private slots:
    void onButtonHovered() {
        logoutLabel->setStyleSheet("color:#1DA1F2;");
    }

    void onButtonLeft() {
        logoutLabel->setStyleSheet("color:#000000;");
    }
};

class BackButton : public QVBoxLayout {
    Q_OBJECT

public:
    CustomButton* logoutButton;
    QLabel* logoutLabel;
    QSize logoutButtonSize;
    QSize hoveredsize;
    QFont size;
    QFont sizeH;

    BackButton(QWidget* parent = nullptr) : QVBoxLayout(parent) {
        QPixmap logoutnormal(":/TRSVote/styles/image/backnormal.png");
        logoutButton = new CustomButton(logoutnormal);
        logoutButton->p_hover.load(":/TRSVote/styles/image/backnormal.png");
        logoutButton->p_clicked.load(":/TRSVote/styles/image/backnormal.png");
        logoutButton->p_disabled.load(":/TRSVote/styles/image/backnormal.png");
        size = QFont("Segoe UI", 12, 600);
        sizeH = QFont("Segoe UI", 12, 800);
        logoutButtonSize = QSize(100, 50);
        hoveredsize = QSize(120, 60);
        logoutButton->resize_btn(logoutButtonSize);
        logoutLabel = new QLabel("Back");
        logoutLabel->setFont(size);
        logoutLabel->setAlignment(Qt::AlignCenter);
        addWidget(logoutButton);
        addWidget(logoutLabel);
        setAlignment(Qt::AlignTop);

        logoutButton->installEventFilter(this);
    }

    void hideButton() {
        logoutButton->hide();
        logoutLabel->hide();
    }

    void showButton() {
        logoutButton->show();
        logoutLabel->show();
    }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (obj == logoutButton) {
            if (event->type() == QEvent::Enter) {
                onButtonHovered();
                logoutButton->resize_btn(hoveredsize);
                logoutLabel->setFont(sizeH);
            }
            else if (event->type() == QEvent::Leave) {
                onButtonLeft();
                logoutButton->resize_btn(logoutButtonSize);
                logoutLabel->setFont(size);
            }
        }

        return false;
    }

private slots:
    void onButtonHovered() {
        logoutLabel->setStyleSheet("color:#1DA1F2;");
    }

    void onButtonLeft() {
        logoutLabel->setStyleSheet("color:#000000;");
    }
};

class HistoryButton : public QVBoxLayout {
    Q_OBJECT

public:
    CustomButton* logoutButton;
    QLabel* logoutLabel;
    QSize logoutButtonSize;
    QSize hoveredsize;
    QFont size;
    QFont sizeH;

    HistoryButton(QWidget* parent = nullptr) : QVBoxLayout(parent) {
        QPixmap logoutnormal(":/TRSVote/styles/image/historybutton.png");
        logoutButton = new CustomButton(logoutnormal);
        logoutButton->p_hover.load(":/TRSVote/styles/image/historybutton.png");
        logoutButton->p_clicked.load(":/TRSVote/styles/image/historybutton.png");
        logoutButton->p_disabled.load(":/TRSVote/styles/image/historybutton.png");
        size = QFont("Segoe UI", 12, 600);
        sizeH = QFont("Segoe UI", 12, 800);
        logoutButtonSize = QSize(100, 50);
        hoveredsize = QSize(120, 60);
        logoutButton->resize_btn(logoutButtonSize);
        logoutLabel = new QLabel("History");
        logoutLabel->setFont(size);
        logoutLabel->setAlignment(Qt::AlignCenter);
        addWidget(logoutButton);
        addWidget(logoutLabel);
        setAlignment(Qt::AlignTop);

        logoutButton->installEventFilter(this);
    }

    void hideButton() {
        logoutButton->hide();
        logoutLabel->hide();
    }

    void showButton() {
        logoutButton->show();
        logoutLabel->show();
    }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (obj == logoutButton) {
            if (event->type() == QEvent::Enter) {
                onButtonHovered();
                logoutButton->resize_btn(hoveredsize);
                logoutLabel->setFont(sizeH);
            }
            else if (event->type() == QEvent::Leave) {
                onButtonLeft();
                logoutButton->resize_btn(logoutButtonSize);
                logoutLabel->setFont(size);
            }
        }

        return false;
    }

private slots:
    void onButtonHovered() {
        logoutLabel->setStyleSheet("color:#1DA1F2;");
    }

    void onButtonLeft() {
        logoutLabel->setStyleSheet("color:#000000;");
    }
};

class CreateVotepollWidget : public QWidget {
    Q_OBJECT
public:
    QVBoxLayout* mainLayout;
    QLabel* titleLabel;
    QVBoxLayout* descLayout;
    QHBoxLayout* desc2Layout;
    QLabel* labelDesc;
    QLineEdit* lineEditDesc;
    QHBoxLayout* dateLayout;
    QVBoxLayout* startDateLayout;
    QLabel* labelStart;
    QDateTimeEdit* dateTimeEditStart;
    QVBoxLayout* endDateLayout;
    QLabel* labelEnd;
    QDateTimeEdit* dateTimeEditEnd;
    QHBoxLayout* sizeLayout;
    QVBoxLayout* CandidateSizeLayout;
    QVBoxLayout* ParticipantSizeLayout;
    QVBoxLayout* RingSizeLayout;
    QLabel* labelSizeCandidate;
    QSpinBox* spinBoxSizeCandidate;
    QLabel* labelSizeParticipant;
    QSpinBox* spinBoxSizeParticipant;
    QLabel* labelRingSize;
    QSpinBox* spinBoxRingSize;
    QHBoxLayout* buttonLayout;
    QPushButton* pushButtonSend;
    CreateVotepollWidget(QWidget* parent = nullptr) : QWidget(parent) {
        mainLayout = new QVBoxLayout;
        QFont big("Segoe UI", 18, 800, false);
        QFont small("Segoe UI", 11, 600, false);
        titleLabel = new QLabel("Create Polling Form");
        titleLabel->setAlignment(Qt::AlignCenter);
        descLayout = new QVBoxLayout;
        labelDesc = new QLabel("Description :");
        labelDesc->setAlignment(Qt::AlignLeft);
        lineEditDesc = new QLineEdit;
        desc2Layout = new QHBoxLayout;
        desc2Layout->addWidget(lineEditDesc);
        desc2Layout->addStretch();
        desc2Layout->setAlignment(Qt::AlignLeft);
        descLayout->addWidget(labelDesc);
        descLayout->addLayout(desc2Layout);
        labelStart = new QLabel("Set Start Date :");
        dateTimeEditStart = new QDateTimeEdit;
        labelEnd = new QLabel("Set End Date :");
        dateTimeEditEnd = new QDateTimeEdit;
        dateLayout = new QHBoxLayout;
        startDateLayout = new QVBoxLayout;
        endDateLayout = new QVBoxLayout;
        startDateLayout->setAlignment(Qt::AlignLeft);
        endDateLayout->setAlignment(Qt::AlignLeft);
        dateLayout->addLayout(startDateLayout);
        dateLayout->addSpacing(20);
        dateLayout->addLayout(endDateLayout);
        dateLayout->setAlignment(Qt::AlignLeft);
        startDateLayout->addWidget(labelStart);
        startDateLayout->addWidget(dateTimeEditStart);
        endDateLayout->addWidget(labelEnd);
        endDateLayout->addWidget(dateTimeEditEnd);
        sizeLayout = new QHBoxLayout;
        CandidateSizeLayout = new QVBoxLayout;
        ParticipantSizeLayout = new QVBoxLayout;
        RingSizeLayout = new QVBoxLayout;
        sizeLayout->addLayout(CandidateSizeLayout);
        sizeLayout->addSpacing(20);
        sizeLayout->addLayout(ParticipantSizeLayout);
        sizeLayout->addSpacing(20);
        sizeLayout->addLayout(RingSizeLayout);
        sizeLayout->addStretch();
        labelSizeCandidate = new QLabel("Candidate Size :");
        labelSizeParticipant = new QLabel("Voter Size :");
        labelRingSize = new QLabel("Ring Size :      ");
        spinBoxSizeCandidate = new QSpinBox;
        spinBoxSizeParticipant = new QSpinBox;
        spinBoxRingSize = new QSpinBox;
        buttonLayout = new QHBoxLayout;
        pushButtonSend = new QPushButton("Create");
        CandidateSizeLayout->addWidget(labelSizeCandidate);
        CandidateSizeLayout->addWidget(spinBoxSizeCandidate);
        ParticipantSizeLayout->addWidget(labelSizeParticipant);
        ParticipantSizeLayout->addWidget(spinBoxSizeParticipant);
        RingSizeLayout->addWidget(labelRingSize);
        RingSizeLayout->addWidget(spinBoxRingSize);
        buttonLayout->addWidget(pushButtonSend);
        buttonLayout->addStretch();
        mainLayout->addSpacing(25);
        mainLayout->addWidget(titleLabel);
        mainLayout->addSpacing(50);
        mainLayout->addLayout(descLayout);
        mainLayout->addSpacing(15);
        mainLayout->addLayout(dateLayout);
        mainLayout->addSpacing(15);
        mainLayout->addLayout(sizeLayout);
        mainLayout->addSpacing(20);
        mainLayout->addLayout(buttonLayout);
        mainLayout->setAlignment(Qt::AlignTop);
        spinBoxSizeCandidate->setMinimum(2);
        spinBoxSizeCandidate->setMaximum(100);
        spinBoxSizeParticipant->setMinimum(4);
        spinBoxSizeParticipant->setMaximum(10000000);
        spinBoxRingSize->setMinimum(2);
        spinBoxRingSize->setMaximum(128);
        QString styles = "QDateTimeEdit {"
            "    border: 1px solid #E1E8ED;"
            "    background-color: #F5F8FA;"
            "    color: #1C2938;"
            "    font-size: 14px;"
            "    padding: 8px 12px;"
            "    border-radius: 20px;"
            "}"
            "QDateTimeEdit:focus {"
            "    border: 1px solid #1DA1F2;"
            "    background-color: #FFFFFF;"
            "    color: #1C2938;"
            "    font-size: 14px;"
            "    padding: 8px 12px;"
            "    border-radius: 20px;"
            "}"
            "QPushButton {"
            "   background-color: #1DA1F2;"
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #0D95E8;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0B7BC0;"
            "}"
            "QSpinBox {"
            "    border: 1px solid #E1E8ED;"
            "    background-color: #F5F8FA;"
            "    color: #1C2938;"
            "    font-size: 14px;"
            "    padding: 8px 12px;"
            "    border-radius: 20px;"
            "}"
            "QSpinBox::up-button {"
            "    subcontrol-origin: border;"
            "    subcontrol-position: top right;"
            "    width: 25px;"
            "    border-top-right-radius: 20px;"
            "    icon: qproperty-icon: updown_arrow;"
            "}"
            "QSpinBox::down-button {"
            "    subcontrol-origin: border;"
            "    subcontrol-position: bottom right;"
            "    width: 25px;"
            "    border-bottom-right-radius: 20px;"
            "    icon: qproperty-icon: updown_arrow;"
            "}"
            "QSpinBox:focus {"
            "    border: 1px solid #1DA1F2;"
            "    background-color: #FFFFFF;"
            "    color: #1C2938;"
            "    font-size: 14px;"
            "    padding: 8px 12px;"
            "    border-radius: 20px;"
            "}";
        setStyleSheet(styles);
        labelDesc->setFont(small);
        labelStart->setFont(small);
        labelEnd->setFont(small);
        labelSizeCandidate->setFont(small);
        labelSizeParticipant->setFont(small);
        labelRingSize->setFont(small);
        pushButtonSend->setFont(small);
        titleLabel->setFont(big);
        setLayout(mainLayout);
    }

};

class AddDialog : public QDialog
{
    Q_OBJECT

public:
    QLabel* TitleLabel;
    QVBoxLayout* mainLayout;
    QVBoxLayout* candidateNameLayout;
    QLabel* candidateLabel;
    QLineEdit* candidateLineEdit;
    QVBoxLayout* candidatePkLayout;
    QLabel* candidatePkLabel;
    QLineEdit* candidatePkLineEdit;
    QHBoxLayout* roleLayout;
    QHBoxLayout* buttonLayout;
    QPushButton* button;
    QPushButton* button2;
    QString hvp;
    int method;
    QComboBox* roleComboBox;
    //QVBoxLayout* roleMainLayout;
    QLabel* roleLabel;
    AddDialog(QWidget* parent = nullptr) : QDialog(parent)
    {
        //roleMainLayout = new QVBoxLayout;
        roleLabel = new QLabel("Role :");
        roleLayout = new QHBoxLayout;
        //roleMainLayout->addWidget(roleLabel);
        //roleMainLayout->addLayout(roleLayout);
        roleComboBox = new QComboBox;
        roleLayout->addWidget(roleLabel);
        roleLayout->addWidget(roleComboBox);
        roleLayout->addStretch();
        roleLayout->setAlignment(Qt::AlignLeft);
        roleComboBox->setStyleSheet(
            "QComboBox {"
            "    border: 1px solid #E1E8ED;"
            "    background-color: #F5F8FA;"
            "    color: #1C2938;"
            "    padding: 8px 12px;"
            "    border-radius: none;"
            "}"
            "QComboBox::drop-down {"
            "    subcontrol-origin: padding;"
            "    subcontrol-position: center right;"
            "    width: 25px;"
            "    background-color: #E1E8ED;"
            "    border-top-right-radius: 20px;"
            "    border-bottom-right-radius: 20px;"
            "}"
            "QComboBox::down-arrow {"
            "    image: url(:/TRSVote/styles/image/downarrow.png);"
            "    width: 21px;"
            "    height: 21px;"
            "}"
            "QComboBox::down-arrow:on {"
            "    top: 1px;"
            "}"
            "QComboBox::down-arrow:pressed {"
            "    top: 2px;"
            "}"
            "QComboBox::drop-down:focus {"
            "    outline: none;"
            "}"
            "QComboBox:focus {"
            "    border: 1px solid #1DA1F2;"
            "    background-color: #FFFFFF;"
            "    color: #1C2938;"
            "    font-size: 14px;"
            "    padding: 8px 12px;"
            "    border-radius: 20px;"
            "}"
            "QComboBox::item {"
            "    background-color: #F5F8FA;"
            "    color: #1C2938;"
            "    padding: 8px 12px;"
            "}"
            "QComboBox::item:selected {"
            "    background-color: #1DA1F2;"
            "    color: #FFFFFF;"
            "}"
            "QComboBox::item:disabled {"
            "    background-color: #F5F8FA;"
            "    color: #C0C0C0;"
            "}"
            "QComboBox::separator {"
            "    height: 1px;"
            "    background-color: #E1E8ED;"
            "}"
            "QComboBox::indicator {"
            "    border: none;"
            "    background-color: transparent;"
            "}");
        roleComboBox->addItem("Candidate");
        roleComboBox->addItem("Voter");
        method = 0;
        TitleLabel = new QLabel("Form Member Registration");
        QFont fontT("Segoe UI", 14, 800, false);
        TitleLabel->setAlignment(Qt::AlignCenter);
        QFont font("Segoe UI", 10, 600, false);
        QString stylesheetButton = "QWidget{background-color: #FFFFFF;}"
            "QPushButton {"
            "   background-color: #1DA1F2;"
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #0D95E8;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0B7BC0;"
            "}"
            "QLineEdit {"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "outline:none;"
            "}"
            "QPushButton:focus { outline: none; }"
            "QLineEdit {"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "}";
        // Create the main layout
        mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(TitleLabel);
        mainLayout->addStretch(1);
        mainLayout->addLayout(roleLayout);
        roleLayout->setContentsMargins(2, 3, 2, 0);
        // Create the layout for candidate name
        candidateNameLayout = new QVBoxLayout();
        mainLayout->addLayout(candidateNameLayout);

        // Create the label for candidate name
        candidateLabel = new QLabel("Candidate Name :");
        candidateNameLayout->addWidget(candidateLabel);

        // Create the line edit for candidate name
        candidateLineEdit = new QLineEdit();
        candidateNameLayout->addWidget(candidateLineEdit);
        candidateNameLayout->setContentsMargins(2, 1, 2, 0);
        // Create the layout for candidate PK
        candidatePkLayout = new QVBoxLayout();
        mainLayout->addLayout(candidatePkLayout);

        // Create the label for candidate PK
        candidatePkLabel = new QLabel("Candidate Publickey :");
        candidatePkLayout->addWidget(candidatePkLabel);

        // Create the line edit for candidate PK
        candidatePkLineEdit = new QLineEdit();
        candidatePkLayout->addWidget(candidatePkLineEdit);
        candidatePkLayout->setContentsMargins(2, 0, 2, 1);
        mainLayout->addStretch(1);
        // Create the layout for the buttons
        buttonLayout = new QHBoxLayout();
        mainLayout->addLayout(buttonLayout);

        // Create the button
        button = new QPushButton("Add");
        button2 = new QPushButton("Cancel");
        buttonLayout->addStretch();
        buttonLayout->addWidget(button);
        buttonLayout->addWidget(button2);
        buttonLayout->setAlignment(Qt::AlignLeft);
        // Set the stylesheet for the cancel button
        button2->setStyleSheet("QPushButton {"
            "   background-color: #FF0000;"  // Red color
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #CC0000;"  // Lighter shade of red on hover
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #990000;"  // Darker shade of red when pressed
            "}");
        setFont(font);
        // Set the main layout for the dialog
        setLayout(mainLayout);
        setStyleSheet(stylesheetButton);
        TitleLabel->setFont(fontT);
        setFixedWidth(500);
        connect(roleComboBox, &QComboBox::currentIndexChanged, [&](int index) {
            if (index == 0) {
                candidateLabel->show();
                candidateLineEdit->show();
                candidatePkLabel->setText("Candidate Publickey :");
            }
            else if (index == 1) {
                candidateLabel->hide();
                candidateLineEdit->hide();
                candidatePkLabel->setText("Voter Publickey :");
            }
            });
    }

    void setupCondition(int a) {
    candidateLineEdit->clear();
    candidatePkLineEdit->clear();
        if (a == 1) {
            TitleLabel->setText("Form Member Registration");
            candidateLabel->show();
            candidateLineEdit->show();
            candidatePkLabel->setText("Candidate Publickey");
            method = 1;
        }else if (a == 2) {
            TitleLabel->setText("Form Participant Registration");
            candidateLabel->hide();
            candidateLineEdit->hide();
            candidatePkLabel->setText("Participant Publickey");
            method = 2;
        }
    }
};

class PollingDetailDialog : public QDialog
{
    Q_OBJECT

public:
    QLabel* TitleLabel;
    QVBoxLayout* mainLayout;
    QHBoxLayout* topLayout;
    QLabel* descLabel;
    QLabel* descData;
    QHBoxLayout* dateLayout;
    QVBoxLayout* startDateLayout;
    QLabel* startDateLabel;
    QLabel* startDateData;
    QVBoxLayout* endDateLayout;
    QLabel* endDateLabel;
    QLabel* endDateData;
    QVBoxLayout* blockDateLayout;
    QLabel* blockDateLabel;
    QLabel* blockDateData;
    QHBoxLayout* sizeLayout;
    QVBoxLayout* candidateLayout;
    QLabel* candidateLabel;
    QLabel* candidateDataLabel;
    QLabel* candidateMaxLabel;
    QVBoxLayout* participantLayout;
    QLabel* participantLabel;
    QLabel* participantDataLabel;
    QLabel* participantMaxLabel;
    QVBoxLayout* ringLayout;
    QLabel* ringLabel;
    QLabel* ringDataLabel;
    QHBoxLayout* bottomLayout;
    QVBoxLayout* labelLayout;
    QLabel* pkLabel;
    QLabel* sigLabel;
    QVBoxLayout* lineEditLayout;
    QLineEdit* pkLineEdit;
    QPlainTextEdit* sigLineEdit;
    QString hvp;
    PollingDetailDialog(QWidget* parent = nullptr) : QDialog(parent)
    {
        QIcon iconui(":/TRSVote/styles/image/dialogico.png");
        setWindowIcon(iconui);
        topLayout = new QHBoxLayout;
        descLabel = new QLabel("Description :");
        descData = new QLabel("Something like votepoll description here");
        topLayout->addWidget(descLabel);
        topLayout->addWidget(descData);
        topLayout->setAlignment(Qt::AlignLeft);
        TitleLabel = new QLabel("Polling Detail");
        QFont fontT("Segoe UI", 14, 800, false);
        TitleLabel->setAlignment(Qt::AlignCenter);
        QFont font("Segoe UI", 10, 600, false);
        mainLayout = new QVBoxLayout();
        dateLayout = new QHBoxLayout();
        startDateLayout = new QVBoxLayout();
        startDateLabel = new QLabel("Start Date : ");
        QDateTime date = QDateTime::currentDateTime();
        startDateData = new QLabel(date.toString("h:mm AP\nddd, M/d/yyyy"));
        startDateLayout->addWidget(startDateLabel);
        startDateLayout->addWidget(startDateData);

        endDateLayout = new QVBoxLayout();
        endDateLabel = new QLabel("End Date : ");
        endDateData = new QLabel(date.toString("h:mm AP\nddd, M/d/yyyy"));
        endDateLayout->addWidget(endDateLabel);
        endDateLayout->addWidget(endDateData);
        blockDateLayout = new QVBoxLayout();
        blockDateLabel = new QLabel("Created Date :");
        blockDateData = new QLabel(date.toString("h:mm AP\nddd, M/d/yyyy"));
        blockDateLayout->addWidget(blockDateLabel);
        blockDateLayout->addWidget(blockDateData);

        sizeLayout = new QHBoxLayout();
        candidateLayout = new QVBoxLayout();
        candidateLabel = new QLabel("Candidate Size :");
        candidateDataLabel = new QLabel("Current : 2");
        candidateLayout->addWidget(candidateLabel);
        candidateLayout->addWidget(candidateDataLabel);
        candidateMaxLabel = new QLabel("Maximum : 20");
        candidateLayout->addWidget(candidateMaxLabel);
        participantLayout = new QVBoxLayout();
        participantLabel = new QLabel("Participant Size :");
        participantDataLabel = new QLabel("Current : 2000000 ");
        participantMaxLabel = new QLabel("Maximum : 2000000");
        participantLayout->addWidget(participantLabel);
        participantLayout->addWidget(participantDataLabel);
        participantLayout->addWidget(participantMaxLabel);
        ringLayout = new QVBoxLayout();
        ringLabel = new QLabel("Ring Size :");
        ringDataLabel = new QLabel("128");
        ringDataLabel->setAlignment(Qt::AlignTop);
        ringLayout->addWidget(ringLabel);
        ringLayout->addWidget(ringDataLabel);
        ringLayout->addStretch();
        bottomLayout = new QHBoxLayout();
        labelLayout = new QVBoxLayout();
        pkLabel = new QLabel("Publickey :");
        sigLabel = new QLabel("Signature :");
        labelLayout->addWidget(pkLabel);
        labelLayout->addWidget(sigLabel);
        lineEditLayout = new QVBoxLayout();
        pkLineEdit = new QLineEdit();
        pkLineEdit->setText("1234567812345678123456781234567812345678123456781234567812345678");
        sigLineEdit = new QPlainTextEdit();
        sigLineEdit->setPlainText("12345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678");
        lineEditLayout->addWidget(pkLineEdit);
        lineEditLayout->addWidget(sigLineEdit);
        lineEditLayout->addStretch();
        dateLayout->addLayout(startDateLayout);
        dateLayout->addLayout(endDateLayout);
        dateLayout->addLayout(blockDateLayout);
        sizeLayout->addLayout(candidateLayout);
        sizeLayout->addLayout(participantLayout);
        sizeLayout->addLayout(ringLayout);
        bottomLayout->addLayout(labelLayout);
        bottomLayout->addLayout(lineEditLayout);
        QFrame* separator = new QFrame(this);
        separator->setFrameShape(QFrame::VLine);
        separator->setFrameShadow(QFrame::Sunken);
        QFrame* separator2 = new QFrame(this);
        separator->setFrameShape(QFrame::VLine);
        separator->setFrameShadow(QFrame::Sunken);
        QFrame* separator3 = new QFrame(this);
        separator->setFrameShape(QFrame::VLine);
        separator->setFrameShadow(QFrame::Sunken);
        mainLayout->addWidget(TitleLabel);
        mainLayout->addStretch();
        mainLayout->addLayout(topLayout);
        mainLayout->addStretch();
        mainLayout->addWidget(separator);
        mainLayout->addLayout(dateLayout);
        mainLayout->addStretch();
        mainLayout->addWidget(separator2);
        mainLayout->addLayout(sizeLayout);
        mainLayout->addStretch();
        mainLayout->addWidget(separator3);
        mainLayout->addLayout(bottomLayout);
        mainLayout->addStretch();
        setFont(font);
        setLayout(mainLayout);
        TitleLabel->setFont(fontT);
        setStyleSheet("QWidget{background-color: #FFFFFF;}"
            "QLineEdit{"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QLineEdit:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QLineEdit[readOnly=\"true\"] {"
            "   background-color: #F5F8FA;"
            "   color: #657786;"
            "   border: 1px;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QPlainTextEdit {"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QPlainTextEdit:disabled {"
            "   background-color: #F5F8FA;"
            "   color: #AAB8C2;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QPlainTextEdit[readOnly=\"true\"] {"
            "   background-color: #F5F8FA;"
            "   color: #657786;"
            "   border: 1px;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}");
        pkLineEdit->setReadOnly(true);
        sigLineEdit->setReadOnly(true);
        setFixedWidth(650);
        setFixedHeight(350);
    }

    ~PollingDetailDialog()
    {
        delete TitleLabel;
        delete startDateLabel;
        delete startDateData;
        delete endDateLabel;
        delete endDateData;
        delete blockDateLabel;
        delete blockDateData;
        delete candidateLabel;
        delete candidateDataLabel;
        delete participantLabel;
        delete participantDataLabel;
        delete ringLabel;
        delete ringDataLabel;
        delete pkLabel;
        delete sigLabel;
        delete pkLineEdit;
        delete sigLineEdit;
        delete labelLayout;
        delete startDateLayout;
        delete endDateLayout;
        delete blockDateLayout;
        delete candidateLayout;
        delete participantLayout;
        delete ringLayout;
        delete sizeLayout;
        delete lineEditLayout;
        delete bottomLayout;
        delete dateLayout;
        delete mainLayout;
    }
};

class RegistrationForm : public QDialog
{
    Q_OBJECT
public:
    QVBoxLayout* mainLayout;
    QLabel* titleLabel;
    QHBoxLayout* nameLayout;
    QLabel* nameLabel;
    QLineEdit* nameLineEdit; 
    QHBoxLayout* idCardLayout;
    QLabel* idCardLabel;
    QLineEdit* idCardLineEdit; 
    QHBoxLayout* affiliateLayout;
    QLabel* affiliateLabel;
    QLineEdit* affiliateLineEdit;
    QHBoxLayout* roleLayout;
    QComboBox* roleComboBox;
    QHBoxLayout* descLayout;
    QLabel* descLabel;
    QPlainTextEdit* descLineEdit;
    QHBoxLayout* pushButtonLayout;
    QPushButton* sendButton;
    QPushButton* cancelButton;
    QString hvp;
    int method;
    RegistrationForm(QWidget* parent = nullptr) : QDialog(parent)
    {
        affiliateLayout = new QHBoxLayout;
        affiliateLabel = new QLabel("Affiliate :");
        affiliateLabel->setAlignment(Qt::AlignLeft);
        affiliateLineEdit = new QLineEdit;
        affiliateLineEdit->setAlignment(Qt::AlignLeft);
        affiliateLayout->addWidget(affiliateLabel);
        affiliateLayout->addSpacing(21);
        affiliateLayout->addWidget(affiliateLineEdit);
        QFont fontT("Segoe UI", 18, 800, false);
        QFont font("Segoe UI", 10, 600, false);
        method = 0;
        mainLayout = new QVBoxLayout(this);
        titleLabel = new QLabel("Form Registration");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        mainLayout->addSpacing(20);

        nameLayout = new QHBoxLayout;
        nameLabel = new QLabel("Name :");
        nameLineEdit = new QLineEdit;
        nameLayout->addWidget(nameLabel);
        nameLayout->addSpacing(30);
        nameLabel->setAlignment(Qt::AlignLeft);
        nameLayout->addWidget(nameLineEdit);
        nameLineEdit->setAlignment(Qt::AlignLeft);
        mainLayout->addLayout(nameLayout);
        
        idCardLayout = new QHBoxLayout;
        idCardLabel = new QLabel("ID Card :");
        idCardLineEdit = new QLineEdit;
        idCardLayout->addWidget(idCardLabel);
        idCardLabel->setAlignment(Qt::AlignLeft);
        idCardLayout->addSpacing(22);
        idCardLayout->addWidget(idCardLineEdit);
        idCardLineEdit->setAlignment(Qt::AlignLeft);
        mainLayout->addLayout(idCardLayout);

        roleLayout = new QHBoxLayout;
        roleComboBox = new QComboBox;
        roleComboBox->addItem("Choose Role");
        roleComboBox->addItem("Candidate");
        roleComboBox->addItem("Voter");
        roleLayout->addSpacing(75);
        roleLayout->addWidget(roleComboBox);
        roleLayout->setAlignment(Qt::AlignLeft);
        roleLayout->addStretch();
        mainLayout->addLayout(affiliateLayout);
        descLayout = new QHBoxLayout;
        QVBoxLayout* desclabelLayout = new QVBoxLayout;
        descLabel = new QLabel("More\nInformation :");
        descLineEdit = new QPlainTextEdit;
        desclabelLayout->addWidget(descLabel);
        desclabelLayout->addStretch();
        desclabelLayout->setAlignment(Qt::AlignTop);
        descLayout->addLayout(desclabelLayout);
        descLayout->addWidget(descLineEdit);
        mainLayout->addLayout(roleLayout);
        roleComboBox->setStyleSheet(
            "QComboBox {"
            "    border: 1px solid #E1E8ED;"
            "    background-color: #F5F8FA;"
            "    color: #1C2938;"
            "    padding: 8px 12px;"
            "    border-radius: none;"
            "}"
            "QComboBox::drop-down {"
            "    subcontrol-origin: padding;"
            "    subcontrol-position: center right;"
            "    width: 25px;"
            "    background-color: #E1E8ED;"
            "    border-top-right-radius: 20px;"
            "    border-bottom-right-radius: 20px;"
            "}"
            "QComboBox::down-arrow {"
            "    image: url(:/TRSVote/styles/image/downarrow.png);"
            "    width: 21px;"
            "    height: 21px;"
            "}"
            "QComboBox::down-arrow:on {"
            "    top: 1px;"
            "}"
            "QComboBox::down-arrow:pressed {"
            "    top: 2px;"
            "}"
            "QComboBox::drop-down:focus {"
            "    outline: none;"
            "}"
            "QComboBox:focus {"
            "    border: 1px solid #1DA1F2;"
            "    background-color: #FFFFFF;"
            "    color: #1C2938;"
            "    font-size: 14px;"
            "    padding: 8px 12px;"
            "    border-radius: 20px;"
            "}"
            "QComboBox::item {"
            "    background-color: #F5F8FA;"
            "    color: #1C2938;"
            "    padding: 8px 12px;"
            "}"
            "QComboBox::item:selected {"
            "    background-color: #1DA1F2;"
            "    color: #FFFFFF;"
            "}"
            "QComboBox::item:disabled {"
            "    background-color: #F5F8FA;"
            "    color: #C0C0C0;"
            "}"
            "QComboBox::separator {"
            "    height: 1px;"
            "    background-color: #E1E8ED;"
            "}"
            "QComboBox::indicator {"
            "    border: none;"
            "    background-color: transparent;"
            "}"
        );
        mainLayout->addLayout(descLayout);
        pushButtonLayout = new QHBoxLayout;
        pushButtonLayout->setAlignment(Qt::AlignRight);
        pushButtonLayout->setContentsMargins(2, 2, 2, 2);
        sendButton = new QPushButton("Send Request");
        sendButton->setStyleSheet("QPushButton {"
            "   background-color: #1DA1F2;"
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #0D95E8;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0B7BC0;"
            "}");
        setStyleSheet("QWidget{background-color: #FFFFFF;}"
            "QLineEdit{"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QLineEdit:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QLineEdit[readOnly=\"true\"] {"
            "   background-color: #F5F8FA;"
            "   color: #657786;"
            "   border: 1px;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QPlainTextEdit {"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QPlainTextEdit:disabled {"
            "   background-color: #F5F8FA;"
            "   color: #AAB8C2;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QPlainTextEdit[readOnly=\"true\"] {"
            "   background-color: #F5F8FA;"
            "   color: #657786;"
            "   border: 1px;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}");
        cancelButton = new QPushButton("Cancel");
        cancelButton->setStyleSheet("QPushButton {"
            "   background-color: #FF0000;"  // Red color
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #CC0000;"  // Lighter shade of red on hover
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #990000;"  // Darker shade of red when pressed
            "}");
        pushButtonLayout->addStretch();
        pushButtonLayout->addWidget(sendButton);
        pushButtonLayout->addWidget(cancelButton);
        mainLayout->addLayout(pushButtonLayout);
        setFont(font);
        titleLabel->setFont(fontT);
        setLayout(mainLayout);
        setFixedWidth(500);
        setFixedHeight(400);
    }

    ~RegistrationForm()
    {
        delete titleLabel;
        delete nameLabel;
        delete nameLineEdit;
        delete idCardLabel;
        delete idCardLineEdit;
        delete descLabel;
        delete descLineEdit;
        delete sendButton;
        delete cancelButton;
        delete nameLayout;
        delete idCardLayout;
        delete pushButtonLayout;
        delete descLayout;
        delete mainLayout;
    }
};

class MessageListWidget : public QWidget {
    Q_OBJECT

public:
    QByteArray cipher;
    QString pk;
    QString hvp;
    QString desc;
    QVBoxLayout* mainLayout;
    QLabel* name;
    QLabel* role;
    long long blocktime;
    long long acceptedtime;
    long long hvpstartdate;
    int irole;
    QString qname;
    QString qid;
    QString qaf;
    QString qinf;
    MessageListWidget(QWidget* parent = nullptr) : QWidget(parent) {
        hvpstartdate = 0;
        acceptedtime = 0;
        irole = 0;
        QFont bold("Segoe UI", 11, 800, false);
        QFont n("Segoe UI", 9, 450, false);
        blocktime = 0;
        mainLayout = new QVBoxLayout;
        name = new QLabel("Some Name");
        name->setFont(bold);
        role = new QLabel("Want to become Voter in Test Polling");
        role->setFont(n);
        mainLayout->addWidget(name);
        mainLayout->addWidget(role);
        setLayout(mainLayout);
        mainLayout->setAlignment(Qt::AlignLeft);
        setStyleSheet(
            "background-color:none;"
        );
    }

    void openCipher(uint8_t* privatekey) {
        long long cipherlen = cipher.size();
        if (cipherlen < 11) {
            return;
        }
        cipherlen -= 28;
        AES_Data c;
        c.ciphertext = new uint8_t[cipherlen]{};
        memcpy(c.ciphertext, cipher.data(), cipherlen);
        memcpy(c.tag, cipher.data() + cipherlen, 16);
        c.cipherlen = cipherlen;
        cipherlen += 16;
        memcpy(c.nonce, cipher.data() + cipherlen, 12);
        uint8_t* msg = nullptr;
        uint8_t upk[32]{};
        Utility::hexstring_to_uint8_t(upk, 32, pk.toStdString());
        aes_decrypt(privatekey, upk, &c, msg);
        irole = Utility::uint8_t_to_int(msg, 2);
        int size_n = Utility::uint8_t_to_int(msg + 2, 2);
        int size_id = Utility::uint8_t_to_int(msg + 4, 2);
        int size_af = Utility::uint8_t_to_int(msg + 6, 2);
        int size_in = Utility::uint8_t_to_int(msg + 8, 2);
        int pos = 10;
        qname = Utility::uint8_t_to_ascii(msg + pos, size_n).c_str();
        pos += size_n;
        qid = Utility::uint8_t_to_ascii(msg + pos, size_id).c_str();
        pos += size_id;
        qaf = Utility::uint8_t_to_ascii(msg + pos, size_af).c_str();
        pos += size_af;
        qinf = Utility::uint8_t_to_ascii(msg + pos, size_in).c_str();
        name->setText(qname);
        updateAccepted();
        long long currentDate = QDateTime::currentSecsSinceEpoch();
        if (currentDate > hvpstartdate) {
            setDisabled(true);
        }
        delete[] msg;
    }

    void openCipherAsUser(Block::KeyPair* key, uint8_t* gensispk) {
        long long cipherlen = cipher.size();
        if (cipherlen < 11) {
            return;
        }
        cipherlen -= 28;
        AES_Data c;
        c.ciphertext = new uint8_t[cipherlen]{};
        memcpy(c.ciphertext, cipher.data(), cipherlen);
        memcpy(c.tag, cipher.data() + cipherlen, 16);
        c.cipherlen = cipherlen;
        cipherlen += 16;
        memcpy(c.nonce, cipher.data() + cipherlen, 12);
        uint8_t* msg = nullptr;
        aes_decrypt(key->privatekey, gensispk, &c, msg);
        irole = Utility::uint8_t_to_int(msg, 2);
        int size_n = Utility::uint8_t_to_int(msg + 2, 2);
        int size_id = Utility::uint8_t_to_int(msg + 4, 2);
        int size_af = Utility::uint8_t_to_int(msg + 6, 2);
        int size_in = Utility::uint8_t_to_int(msg + 8, 2);
        int pos = 10;
        qname = Utility::uint8_t_to_ascii(msg + pos, size_n).c_str();
        pos += size_n;
        qid = Utility::uint8_t_to_ascii(msg + pos, size_id).c_str();
        pos += size_id;
        qaf = Utility::uint8_t_to_ascii(msg + pos, size_af).c_str();
        pos += size_af;
        qinf = Utility::uint8_t_to_ascii(msg + pos, size_in).c_str();
        name->setText(qname);
        long long currentDate = QDateTime::currentSecsSinceEpoch();
        delete[] msg;
    }


    void updateTime() {
        long long currentDate = QDateTime::currentSecsSinceEpoch();
        if (currentDate > hvpstartdate) {
            setDisabled(true);
        }
    }

    void updateAccepted() {
        if (acceptedtime != 0) {
            if (irole == 1) {
                role->setText("Already become candidate in " + desc);
            }
            else if (irole == 2) {
                role->setText("Already become voter in " + desc);
            }
            setDisabled(true);
        }
        else {
            if (irole == 1) {
                role->setText("Want to become candidate in " + desc);
            }
            else if (irole == 2) {
                role->setText("Want to become voter in " + desc);
            }
            setDisabled(false);
        }
    }
};

class MailWidget : public QWidget
{
    Q_OBJECT
public:
    QGridLayout* centralLayout;
    QStackedWidget* mainStack;
    QWidget* mainWidget;
    QVBoxLayout* pleaseWaitLayout;
    QFrame* pleaseWaitFrame;
    QLabel* pleaseWaitLabel;
    QVBoxLayout* mainLayout;
    QHBoxLayout* mainMailLayout;
    QLabel* titleLabel;
    QListWidget* listWidget;
    std::unordered_map<std::string, MessageListWidget*> set_list_theme;
    std::unordered_map<std::string, QListWidgetItem*> set_list_item;
    QVector<std::string> set_list_hvppk;
    QVBoxLayout* mailLayout;
    QHBoxLayout* headerMailLayout;
    QVBoxLayout* labelLayout;
    QLabel* pkLabel;
    QLabel* nameLabel;
    QLabel* idLabel;
    QLabel* afLabel;
    QLabel* roleLabel;
    QVBoxLayout* dataLayout;
    QLineEdit* pkLineEdit;
    QLineEdit* nameLineEdit;
    QLineEdit* idLineEdit;
    QLineEdit* afLineEdit;
    QPlainTextEdit* textEdit;
    QHBoxLayout* footerMailLayout;
    QLabel* dateLabel;
    QPushButton* acceptButton;
    QLabel* closedRegistrationLabel;
    QLabel* maxUserLabel;
    QFrame* mailFrame;
    std::string hvp;
    MailWidget(QWidget* parent = nullptr) : QWidget(parent) {
        centralLayout = new QGridLayout;
        mainStack = new QStackedWidget;
        centralLayout->addWidget(mainStack);
        pleaseWaitFrame = new QFrame;
        pleaseWaitLayout = new QVBoxLayout;
        pleaseWaitLabel = new QLabel("Please Wait...");
        pleaseWaitFrame->setFrameShape(QFrame::Box);
        pleaseWaitFrame->setFrameShadow(QFrame::Sunken);
        pleaseWaitFrame->setLayout(pleaseWaitLayout);
        pleaseWaitLabel->setAlignment(Qt::AlignCenter);
        pleaseWaitLayout->setAlignment(Qt::AlignCenter);
        pleaseWaitLayout->addWidget(pleaseWaitLabel);
        mainStack->addWidget(pleaseWaitFrame);
        
        closedRegistrationLabel = new QLabel("Registration has been closed");
        maxUserLabel = new QLabel();
        mainLayout = new QVBoxLayout;
        QFont fontT("Segoe UI", 18, 800, false);
        pleaseWaitLabel->setFont(fontT);
        QFont fonts("Segoe UI", 9, 600, false);
        titleLabel = new QLabel("Mail");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        mainLayout->addSpacing(30);
        mailFrame = new QFrame;
        mailFrame->setFrameShape(QFrame::Box);
        mailFrame->setFrameShadow(QFrame::Sunken);
        dateLabel = new QLabel("");
        
        mainMailLayout = new QHBoxLayout();
        listWidget = new QListWidget();
        listWidget->setFixedWidth(250);
        mailLayout = new QVBoxLayout();
        headerMailLayout = new QHBoxLayout();
        labelLayout = new QVBoxLayout();
        dataLayout = new QVBoxLayout();
        textEdit = new QPlainTextEdit();
        textEdit->setReadOnly(true);
        roleLabel = new QLabel("");
        roleLabel->setAlignment(Qt::AlignCenter);
        roleLabel->setFont(fonts);
        roleLabel->setWordWrap(true);
        pkLabel = new QLabel("Publickey :");
        nameLabel = new QLabel("Name :");
        idLabel = new QLabel("ID Card :");
        afLabel = new QLabel("Affiliate :");
        labelLayout->addWidget(pkLabel);
        labelLayout->addWidget(nameLabel);
        labelLayout->addWidget(idLabel);
        labelLayout->addWidget(afLabel);

        pkLineEdit = new QLineEdit;
        nameLineEdit = new QLineEdit;
        idLineEdit = new QLineEdit;
        afLineEdit = new QLineEdit;
        pkLineEdit->setReadOnly(true);
        nameLineEdit->setReadOnly(true);
        idLineEdit->setReadOnly(true);
        afLineEdit->setReadOnly(true);
        dataLayout->addWidget(pkLineEdit);
        dataLayout->addWidget(nameLineEdit);
        dataLayout->addWidget(idLineEdit);
        dataLayout->addWidget(afLineEdit);

        headerMailLayout->addLayout(labelLayout);
        headerMailLayout->addLayout(dataLayout);

        footerMailLayout = new QHBoxLayout();
        dateLabel->setAlignment(Qt::AlignLeft);
        footerMailLayout->addWidget(dateLabel);
        footerMailLayout->addStretch();
        acceptButton = new QPushButton("Accept");
        footerMailLayout->addWidget(acceptButton);
        closedRegistrationLabel->setAlignment(Qt::AlignRight);
        closedRegistrationLabel->hide();
        footerMailLayout->addWidget(closedRegistrationLabel);
        maxUserLabel->setAlignment(Qt::AlignRight);
        maxUserLabel->hide();
        footerMailLayout->addWidget(maxUserLabel);
        mailLayout->addLayout(headerMailLayout);
        mailLayout->addSpacing(10);
        mailLayout->addWidget(textEdit);
        mailLayout->addWidget(roleLabel);
        mailLayout->addSpacing(10);
        mailLayout->addLayout(footerMailLayout);
        mainMailLayout->addWidget(listWidget);
        mailFrame->setLayout(mailLayout);
        mainMailLayout->addWidget(mailFrame);
        mainMailLayout->setContentsMargins(0, 10, 10, 10);
        mainLayout->addLayout(mainMailLayout);
        mainWidget = new QWidget;
        mainWidget->setLayout(mainLayout);
        mainStack->addWidget(mainWidget);
        setStyleSheet("QPushButton {"
            "   background-color: #1DA1F2;"
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #0D95E8;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0B7BC0;"
            "}"
            "QLineEdit{"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QLineEdit:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QLineEdit[readOnly=\"true\"] {"
            "   background-color: #F5F8FA;"
            "   color: #657786;"
            "   border: 1px;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QPlainTextEdit {"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QPlainTextEdit:disabled {"
            "   background-color: #F5F8FA;"
            "   color: #AAB8C2;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QPlainTextEdit[readOnly=\"true\"] {"
            "   background-color: #F5F8FA;"
            "   color: #657786;"
            "   border: 1px;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QPushButton:focus { outline: none; }");
        listWidget->setStyleSheet("QListWidget{border: 2px solid #1DA1F2;}");
        setFont(fonts);
        titleLabel->setFont(fontT);
        setLayout(centralLayout);
        hideMail();
    }

    ~MailWidget() {
    }

    void hideMail() {
        nameLineEdit->setVisible(false);
        idLineEdit->setVisible(false);
        afLineEdit->setVisible(false);
        pkLineEdit->setVisible(false);
        textEdit->setVisible(false);
        nameLabel->setVisible(false);
        idLabel->setVisible(false);
        afLabel->setVisible(false);
        pkLabel->setVisible(false);
        acceptButton->setVisible(false);
        roleLabel->setVisible(false);
        dateLabel->setVisible(false);
    }

    void showMail() {
        nameLineEdit->setVisible(true);
        idLineEdit->setVisible(true);
        afLineEdit->setVisible(true);
        pkLineEdit->setVisible(true);
        textEdit->setVisible(true);
        nameLabel->setVisible(true);
        idLabel->setVisible(true);
        afLabel->setVisible(true);
        pkLabel->setVisible(true);
        acceptButton->setVisible(true);
        roleLabel->setVisible(true);
        dateLabel->setVisible(true);
    }

    void acceptMail() {
        int cur = listWidget->currentRow();
        if (cur < 0) {
            return;
        }
        if (cur >= set_list_hvppk.size()) {
            return;
        }
        std::string key = set_list_hvppk[cur];
        auto find = set_list_theme.find(key);
        if (find == set_list_theme.end()) {
            return;
        }
        long long currentm = QDateTime::currentSecsSinceEpoch() + 150;
        if (find->second->acceptedtime == 0) {
            if (find->second->irole == 1) {
                roleLabel->setText("This user want to become candidate in " + find->second->desc);
            }
            else if (find->second->irole == 2) {
                roleLabel->setText("This user want to become voter in this " + find->second->desc);
            }
            if (currentm > find->second->hvpstartdate) {
                acceptButton->setVisible(false);
                closedRegistrationLabel->show();
                return;
            }
            closedRegistrationLabel->hide();
            acceptButton->setVisible(true);
        }
        else {
            if (find->second->irole == 1) {
                roleLabel->setText("Already become candidate in " + find->second->desc + " at " + QDateTime::fromSecsSinceEpoch(find->second->acceptedtime).toString("h:mm AP - ddd, MM/dd/yyyy"));
            }
            else if (find->second->irole == 2) {
                roleLabel->setText("Already become voter in " + find->second->desc + " at " + QDateTime::fromSecsSinceEpoch(find->second->acceptedtime).toString("h:mm AP - ddd, MM/dd/yyyy"));
            }
            if (currentm > find->second->hvpstartdate) {
                closedRegistrationLabel->setText("Registration has been closed");
                closedRegistrationLabel->show();
            }
            else {
                closedRegistrationLabel->hide();
            }
            acceptButton->setVisible(false);
        }
    }

    void mailClose(long long currentcan, long long maxcan, long long currentvoter, long long maxvoter) {
        int cur = listWidget->currentRow();
        if (cur < 0) {
            return;
        }
        if (cur >= set_list_hvppk.size()) {
            return;
        }
        std::string key = set_list_hvppk[cur];
        auto find = set_list_theme.find(key);
        if (find == set_list_theme.end()) {
            return;
        }
        long long currentm = QDateTime::currentSecsSinceEpoch() + 150;
        if (currentm > find->second->hvpstartdate) {
            acceptButton->setVisible(false);
            closedRegistrationLabel->setText("Registration has been closed");
            closedRegistrationLabel->show();
        }
        else {
            acceptButton->setVisible(true);
            closedRegistrationLabel->hide();
        }

        if (find->second->irole == 1) {
            if (currentcan >= maxcan) {
                closedRegistrationLabel->setText("Candidate has reach maximum");
                acceptButton->setVisible(true);
                closedRegistrationLabel->show();
            }
        }
        else if (find->second->irole == 2) {
            if (currentvoter >= maxvoter) {
                closedRegistrationLabel->setText("Voter has reach maximum");
                acceptButton->setVisible(true);
                closedRegistrationLabel->show();
            }
        }
    }

    void clearMail() {
        for (auto it = set_list_theme.begin(); it != set_list_theme.end(); it++) {
            MessageListWidget* temp = nullptr;
            std::swap(it->second, temp);
            delete temp;
        }
        set_list_theme.clear();
        for (auto it = set_list_item.begin(); it != set_list_item.end(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(temp, it->second);
            delete temp;
        }
        set_list_item.clear();
        set_list_hvppk.clear();
        listWidget->clear();
    }
};

class KeyPairDialog : public QDialog
{
    Q_OBJECT
public:
    QLabel* titleLabel;
    QVBoxLayout* mainLayout;
    QLabel* publickeyLabel;
    QLabel* privatekeyLabel;
    CheckBoxLineEdit* publicLineEdit;
    CheckBoxLineEdit* privateLineEdit;
    QHBoxLayout* bodyLayout;
    QVBoxLayout* labelLayout;
    QVBoxLayout* lineEditLayout;
    KeyPairDialog() {
        publickeyLabel = new QLabel("Publickey");
        publickeyLabel->setAlignment(Qt::AlignLeft);
        privatekeyLabel = new QLabel("Privatekey");
        privatekeyLabel->setAlignment(Qt::AlignLeft);
        publicLineEdit = new CheckBoxLineEdit;
        privateLineEdit = new CheckBoxLineEdit;
        QFont fontT("Segoe UI", 18, 800, false);
        titleLabel = new QLabel("Keypair");
        titleLabel->setAlignment(Qt::AlignCenter);
        QFont font("Segoe UI", 10, 600, false);
        mainLayout = new QVBoxLayout;
        bodyLayout = new QHBoxLayout;
        labelLayout = new QVBoxLayout;
        lineEditLayout = new QVBoxLayout;
        bodyLayout->addLayout(labelLayout);
        bodyLayout->addLayout(lineEditLayout);
        lineEditLayout->addWidget(publickeyLabel);
        lineEditLayout->addLayout(publicLineEdit);
        publicLineEdit->iconButton->hide();
        publicLineEdit->lineEdit->setEchoMode(QLineEdit::Normal);
        lineEditLayout->addWidget(privatekeyLabel);
        lineEditLayout->addLayout(privateLineEdit);
        mainLayout->addWidget(titleLabel);
        mainLayout->addSpacing(15);
        mainLayout->addLayout(bodyLayout);
        setLayout(mainLayout);
        setStyleSheet("QWidget{background-color: #FFFFFF;}"
            "QLineEdit{"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QLineEdit:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QLineEdit[readOnly=\"true\"] {"
            "   background-color: #F5F8FA;"
            "   color: #657786;"
            "   border: 1px;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "   outline: none;"
            "}"
            "QPushButton {"
            "   background-color: #1DA1F2;"
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #0D95E8;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0B7BC0;"
            "}");
        setFont(font);
        titleLabel->setFont(fontT);
        publicLineEdit->lineEdit->setReadOnly(true);
        privateLineEdit->lineEdit->setReadOnly(true);
        setFixedHeight(200);
        setFixedWidth(550);
    }

    ~KeyPairDialog() {
        delete titleLabel;
        delete publicLineEdit;
        delete privateLineEdit;
        delete labelLayout;
        delete lineEditLayout;
        delete bodyLayout;
        delete mainLayout;
    }
};

class HistoryItem : public QWidget {
    Q_OBJECT

public:
    QHBoxLayout* mainLayout;
    QVBoxLayout* detailLayout;
    QLabel* dateLabel;
    QLabel* detailLabel;
    bool registered;
    bool voted;
    bool requested;
    explicit HistoryItem(QWidget* parent = nullptr) : QWidget(parent) {
        QFont fontd("Segoe UI", 12, 600, false);
        registered = false;
        voted = false;
        requested = false;
        dateLabel = new QLabel(" 8:51 AM\nSenin, 6/4/2023");
        detailLabel = new QLabel("Requested as Candidate in Polling test");
        detailLabel->setFont(fontd);
        detailLabel->setWordWrap(true);
        detailLabel->setAlignment(Qt::AlignLeft);
        dateLabel->setAlignment(Qt::AlignRight);
        detailLayout = new QVBoxLayout;
        mainLayout = new QHBoxLayout;
        detailLayout->addWidget(detailLabel);
        mainLayout->addLayout(detailLayout);
        mainLayout->addWidget(dateLabel);
        mainLayout->setAlignment(Qt::AlignVCenter);
        setLayout(mainLayout);
        setStyleSheet("background-color:none;");
    }

    ~HistoryItem() {
        delete detailLabel;
        delete dateLabel;
        delete detailLayout;
        delete mainLayout;
    }
};

class HistoryWidget : public QWidget
{
    Q_OBJECT
public:
    QVBoxLayout* mainLayout;
    QLabel* titleLabel;
    QVBoxLayout* frameLayout;
    QFrame* frameListWidget;
    QListWidget* listWidget;
    std::unordered_map<std::string, HistoryItem*> set_list_theme_req;
    std::unordered_map<std::string, HistoryItem*> set_list_theme_reg;
    std::unordered_map<std::string, HistoryItem*> set_list_theme_vote;
    std::unordered_map<std::string, QListWidgetItem*> set_list_item_req;
    std::unordered_map<std::string, QListWidgetItem*> set_list_item_reg;
    std::unordered_map<std::string, QListWidgetItem*> set_list_item_vote;
    HistoryWidget(QWidget* parent = nullptr) : QWidget(parent) {
        QFont fontT("Segoe UI", 18, 800, false);
        mainLayout = new QVBoxLayout();
        titleLabel = new QLabel("History");
        titleLabel->setFont(fontT);
        titleLabel->setAlignment(Qt::AlignCenter);
        frameListWidget = new QFrame();
        listWidget = new QListWidget();
        mainLayout->addWidget(titleLabel);
        frameLayout = new QVBoxLayout();
        frameLayout->addWidget(listWidget);
        frameListWidget->setLayout(frameLayout);
        mainLayout->addWidget(frameListWidget);
        listWidget->setStyleSheet("QListWidget{border: 2px solid #1DA1F2;}");
        setLayout(mainLayout);
    }

    ~HistoryWidget() {
        delete listWidget;
        delete titleLabel;
        delete frameLayout;
        delete frameListWidget;
        delete mainLayout;
    }

    void logoutUser() {
        for (auto it = set_list_theme_reg.begin(); it != set_list_theme_reg.end(); it++) {
            HistoryItem* temp = nullptr;
            std::swap(temp, it->second);
            delete temp;
        }
        set_list_theme_reg.clear();
        for (auto it = set_list_item_reg.begin(); it != set_list_item_reg.end(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(it->second, temp);
            delete temp;
        }
        set_list_item_reg.clear();
        for (auto it = set_list_theme_req.begin(); it != set_list_theme_req.end(); it++) {
            HistoryItem* temp = nullptr;
            std::swap(temp, it->second);
            delete temp;
        }
        set_list_theme_req.clear();
        for (auto it = set_list_item_req.begin(); it != set_list_item_req.end(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(it->second, temp);
            delete temp;
        }
        set_list_item_req.clear();
        for (auto it = set_list_theme_vote.begin(); it != set_list_theme_vote.end(); it++) {
            HistoryItem* temp = nullptr;
            std::swap(temp, it->second);
            delete temp;
        }
        set_list_theme_vote.clear();
        for (auto it = set_list_item_vote.begin(); it != set_list_item_vote.end(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(it->second, temp);
            delete temp;
        }
        set_list_item_vote.clear();
        listWidget->clear();
    }
};

class ListMemberWidgetItem : public QWidget {
    Q_OBJECT

public:
    QHBoxLayout* mainLayout;
    QLabel* dateLabel;
    QLabel* pkLabel;
    QLabel* nameLabel;
    explicit ListMemberWidgetItem(QWidget* parent = nullptr) : QWidget(parent) {
        QFont font("Segoe UI", 9, 500, false);
        mainLayout = new QHBoxLayout;
        dateLabel = new QLabel(" 10:01 AM\nWed, 6/11/2023");
        dateLabel->setAlignment(Qt::AlignRight);
        dateLabel->setFont(font);
        dateLabel->setWordWrap(true);
        nameLabel = new QLabel("Airlangga Susanto Putra");
        nameLabel->setAlignment(Qt::AlignLeft);
        nameLabel->setFont(font);
        nameLabel->setWordWrap(true);
        pkLabel = new QLabel("1234567812345678123456781234567812345678123456781234567812345678");
        pkLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setFont(font);
        pkLabel->setWordWrap(true);
        mainLayout->addWidget(nameLabel);
        mainLayout->addStretch();
        mainLayout->addWidget(pkLabel);
        mainLayout->addStretch();
        mainLayout->addWidget(dateLabel);
        setLayout(mainLayout);
        setStyleSheet("background-color:none;");
    }
};



class ListMemberWidget : public QWidget
{
    Q_OBJECT
public:
    QVBoxLayout* mainLayout;
    QStackedWidget* mainStack;
    QHBoxLayout* pleaseWaitLayout;
    QLabel* pleaseWaitLabel;
    QFrame* pleaseWaitFrame;
    QLabel* titleLabel;
    QFrame* mainFrame;
    QVBoxLayout* frameLayout;
    QTabWidget* mainTab;
    QWidget* candidateWidget;
    QWidget* voterWidget;
    QVBoxLayout* candidateMainLayout;
    QHBoxLayout* headerListWidgetCandidateLayout;
    QFrame* headerListWidgetCandidateFrame;
    QLabel* nameListWidgetCandidateLabel;
    QLabel* pkListWidgetCandidateLabel;
    QLabel* dateListWidgetCandidateLabel;
    QHBoxLayout* searchListWidgetCandidateLayout;
    QComboBox* searchComboboxCandidate;
    QLineEdit* searchLineEditCandidate;
    QPushButton* searchPushButtonCandidate;
    QVBoxLayout* voterMainLayout;
    QHBoxLayout* headerListWidgetVoterLayout;
    QFrame* headerListWidgetVoterFrame;
    QLabel* pkListWidgetVoterLabel;
    QLabel* dateListWidgetVoterLabel;
    QHBoxLayout* searchListWidgetVoterLayout;
    QLineEdit* searchLineEditVoter;
    QPushButton* searchPushButtonVoter;
    QStackedWidget* candidateStackedMain;
    QStackedWidget* voterStackedMain;
    QFrame* pleaseWaitCandidateFrame;
    QFrame* pleaseWaitVoterFrame;
    QLabel* pleaseWaitLabelCandidate;
    QLabel* pleaseWaitLabelVoter;
    QVBoxLayout* pleaseWaitLayoutCandidate;
    QVBoxLayout* pleaseWaitLayoutVoter;
    QListWidget* candidateListWidget;
    QListWidget* voterListWidget;
    QVector<ListMemberWidgetItem*> set_list_theme_candidate;
    QVector<QListWidgetItem*> set_list_item_candidate;
    QVector<ListMemberWidgetItem*> set_list_theme_voter;
    QVector<QListWidgetItem*> set_list_item_voter;
    QString hvp;
    QVBoxLayout* searchCandidateMainLayout;
    QVBoxLayout* searchVoterMainLayout;
    QPushButton* refreshCandidateButton;
    QPushButton* refreshVoterButton;
    QLabel* searchKeywordCandidateLabel;
    QLabel* searchKeywordVoterLabel;
    ListMemberWidget(QWidget* parent = nullptr) : QWidget(parent) {
        candidateStackedMain = new QStackedWidget;
        voterStackedMain = new QStackedWidget;
        pleaseWaitCandidateFrame = new QFrame;
        pleaseWaitVoterFrame = new QFrame;
        pleaseWaitLabelCandidate = new QLabel("Please Wait...");
        pleaseWaitLabelCandidate->setAlignment(Qt::AlignCenter);
        pleaseWaitLabelVoter = new QLabel("Please Wait...");
        pleaseWaitLabelVoter->setAlignment(Qt::AlignCenter);
        pleaseWaitLayoutCandidate = new QVBoxLayout;
        pleaseWaitLayoutCandidate->setAlignment(Qt::AlignTop);
        pleaseWaitLayoutVoter = new QVBoxLayout;
        pleaseWaitLayoutVoter->setAlignment(Qt::AlignTop);
        pleaseWaitCandidateFrame->setLayout(pleaseWaitLayoutCandidate);
        pleaseWaitLayoutCandidate->addWidget(pleaseWaitLabelCandidate);
        pleaseWaitVoterFrame->setLayout(pleaseWaitLayoutVoter);
        pleaseWaitLayoutVoter->addWidget(pleaseWaitLabelVoter);
        searchCandidateMainLayout = new QVBoxLayout;
        searchVoterMainLayout = new QVBoxLayout;
        refreshCandidateButton = new QPushButton;
        QIcon rcb(":/TRSVote/styles/image/refreshButton.png");
        refreshVoterButton = new QPushButton;
        refreshCandidateButton->setIcon(rcb);
        refreshVoterButton->setIcon(rcb);
        searchKeywordCandidateLabel = new QLabel("2 out of 8 found with keyword blabla");
        searchKeywordCandidateLabel->setAlignment(Qt::AlignCenter);
        searchKeywordVoterLabel = new QLabel("0 out of 9 found with keyword blabla");
        searchKeywordVoterLabel->setAlignment(Qt::AlignCenter);
        mainStack = new QStackedWidget;
        pleaseWaitLayout = new QHBoxLayout;
        pleaseWaitLabel = new QLabel("Please Wait...");
        pleaseWaitLabel->setAlignment(Qt::AlignCenter);
        pleaseWaitLayout->addWidget(pleaseWaitLabel);
        pleaseWaitFrame = new QFrame;
        pleaseWaitFrame->setLayout(pleaseWaitLayout);
        searchComboboxCandidate = new QComboBox;
        searchComboboxCandidate->addItem("Search by name");
        searchComboboxCandidate->addItem("Search by publickey");
        searchLineEditCandidate = new QLineEdit;
        searchPushButtonCandidate = new QPushButton("Search");
        searchLineEditVoter = new QLineEdit;
        searchPushButtonVoter = new QPushButton("Search");
        headerListWidgetCandidateFrame = new QFrame;
        nameListWidgetCandidateLabel = new QLabel("Name");
        nameListWidgetCandidateLabel->setAlignment(Qt::AlignLeft);
        pkListWidgetCandidateLabel = new QLabel("Publickey");
        nameListWidgetCandidateLabel->setAlignment(Qt::AlignCenter);
        dateListWidgetCandidateLabel = new QLabel("Date");
        dateListWidgetCandidateLabel->setAlignment(Qt::AlignRight);
        headerListWidgetVoterFrame = new QFrame;
        pkListWidgetVoterLabel = new QLabel("Publickey");
        pkListWidgetVoterLabel->setAlignment(Qt::AlignLeft);
        dateListWidgetVoterLabel = new QLabel("Date");
        dateListWidgetVoterLabel->setAlignment(Qt::AlignRight);
        searchListWidgetCandidateLayout = new QHBoxLayout;
        searchListWidgetVoterLayout = new QHBoxLayout;
        headerListWidgetCandidateLayout = new QHBoxLayout;
        headerListWidgetVoterLayout = new QHBoxLayout;
        headerListWidgetCandidateLayout->addWidget(nameListWidgetCandidateLabel);
        headerListWidgetCandidateLayout->addStretch();
        headerListWidgetCandidateLayout->addWidget(pkListWidgetCandidateLabel);
        headerListWidgetCandidateLayout->addStretch();
        headerListWidgetCandidateLayout->addWidget(dateListWidgetCandidateLabel);
        headerListWidgetCandidateFrame->setLayout(headerListWidgetCandidateLayout);
        headerListWidgetVoterLayout->addWidget(pkListWidgetVoterLabel);
        headerListWidgetVoterLayout->addStretch();
        headerListWidgetVoterLayout->addWidget(dateListWidgetVoterLabel);
        headerListWidgetVoterFrame->setLayout(headerListWidgetVoterLayout);
        headerListWidgetVoterFrame->setStyleSheet("QFrame{border: 2px solid #1DA1F2;border-bottom: none; outline:none;}");
        headerListWidgetCandidateFrame->setStyleSheet("QFrame{border: 2px solid #1DA1F2;border-bottom: none; outline:none;}");
        pkListWidgetVoterLabel->setStyleSheet("border:none;background-color:none;");
        dateListWidgetVoterLabel->setStyleSheet("border:none;background-color:none;");
        nameListWidgetCandidateLabel->setStyleSheet("border:none;background-color:none;");
        pkListWidgetCandidateLabel->setStyleSheet("border:none;background-color:none;");
        dateListWidgetCandidateLabel->setStyleSheet("border:none;background-color:none;");
        candidateMainLayout = new QVBoxLayout;
        voterMainLayout = new QVBoxLayout;
        candidateListWidget = new QListWidget;
        searchCandidateMainLayout->addLayout(searchListWidgetCandidateLayout);
        candidateMainLayout->addLayout(searchCandidateMainLayout);
        candidateMainLayout->addWidget(headerListWidgetCandidateFrame);
        candidateStackedMain->addWidget(pleaseWaitCandidateFrame);
        candidateStackedMain->addWidget(candidateListWidget);
        candidateStackedMain->setCurrentWidget(candidateListWidget);
        candidateMainLayout->addWidget(candidateStackedMain);
        candidateMainLayout->setSpacing(0);
        voterListWidget = new QListWidget;
        searchVoterMainLayout->addLayout(searchListWidgetVoterLayout);
        voterMainLayout->addLayout(searchVoterMainLayout);
        voterMainLayout->addWidget(headerListWidgetVoterFrame);
        voterStackedMain->addWidget(pleaseWaitVoterFrame);
        voterStackedMain->addWidget(voterListWidget);
        voterMainLayout->addWidget(voterStackedMain);
        voterStackedMain->setCurrentWidget(voterListWidget);
        voterMainLayout->setSpacing(0);
        QFont big("Segoe UI", 18, 800, false);
        pleaseWaitLabel->setFont(big);
        QFont med("Segoe UI", 11, 600, false);
        QFont small("Segoe UI", 10, 600, false);
        pleaseWaitLabelCandidate->setFont(med);
        pleaseWaitLabelVoter->setFont(med);
        mainLayout = new QVBoxLayout;
        titleLabel = new QLabel("Member List from");
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setFont(big);
        mainFrame = new QFrame;
        frameLayout = new QVBoxLayout;
        mainFrame->setLayout(frameLayout);
        mainTab = new QTabWidget;
        frameLayout->addWidget(mainTab);
        mainTab->tabBar()->setDocumentMode(true);
        mainTab->tabBar()->setExpanding(true);
        candidateWidget = new QWidget;
        candidateWidget->setLayout(candidateMainLayout);
        voterWidget = new QWidget;
        voterWidget->setLayout(voterMainLayout);
        mainTab->addTab(candidateWidget, "List Candidate");
        mainTab->addTab(voterWidget, "List Voter");
        mainLayout->addWidget(titleLabel);
        mainStack->addWidget(mainFrame);
        mainStack->addWidget(pleaseWaitFrame);
        mainLayout->addWidget(mainStack);
        setLayout(mainLayout);
        nameListWidgetCandidateLabel->setFont(small);
        pkListWidgetCandidateLabel->setFont(small);
        dateListWidgetCandidateLabel->setFont(small);
        pkListWidgetVoterLabel->setFont(small);
        dateListWidgetVoterLabel->setFont(small);
        mainTab->setStyleSheet("QTabBar::tab { height: 40px;}""QTabBar{outline:none;}""QListWidget{border: 2px solid #1DA1F2; outline:none;}""QTabBar::tab:selected {"
            "background-color: #1DA1F2;" 
            "color: white;"              
            "}");
        mainTab->setFont(med);
        setStyleSheet("QPushButton {"
            "   background-color: #1DA1F2;"
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #0D95E8;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0B7BC0;"
            "}"
            "QLineEdit {"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "}"
            "QPushButton:focus { outline: none; }"
            "QComboBox {"
            "    border: 1px solid #E1E8ED;"
            "    background-color: #F5F8FA;"
            "    color: #1C2938;"
            "    padding: 8px 12px;"
            "    border-radius: none;"
            "}"
            "QComboBox::drop-down {"
            "    subcontrol-origin: padding;"
            "    subcontrol-position: center right;"
            "    width: 25px;"
            "    background-color: #E1E8ED;"
            "    border-top-right-radius: 20px;"
            "    border-bottom-right-radius: 20px;"
            "}"
            "QComboBox::down-arrow {"
            "    image: url(:/TRSVote/styles/image/downarrow.png);"
            "    width: 21px;"
            "    height: 21px;"
            "}"
            "QComboBox::down-arrow:on {"
            "    top: 1px;"
            "}"
            "QComboBox::down-arrow:pressed {"
            "    top: 2px;"
            "}"
            "QComboBox::drop-down:focus {"
            "    outline: none;"
            "}"
            "QComboBox:focus {"
            "    border: 1px solid #1DA1F2;"
            "    background-color: #FFFFFF;"
            "    color: #1C2938;"
            "    font-size: 14px;"
            "    padding: 8px 12px;"
            "    border-radius: 20px;"
            "}"
            "QComboBox::item {"
            "    background-color: #F5F8FA;"
            "    color: #1C2938;"
            "    padding: 8px 12px;"
            "}"
            "QComboBox::item:selected {"
            "    background-color: #1DA1F2;"
            "    color: #FFFFFF;"
            "}"
            "QComboBox::item:disabled {"
            "    background-color: #F5F8FA;"
            "    color: #C0C0C0;"
            "}"
            "QComboBox::separator {"
            "    height: 1px;"
            "    background-color: #E1E8ED;"
            "}"
            "QComboBox::indicator {"
            "    border: none;"
            "    background-color: transparent;"
            "}");
        searchCandidateMainLayout->addWidget(searchKeywordCandidateLabel);
        searchCandidateMainLayout->addSpacing(10);
        searchVoterMainLayout->addWidget(searchKeywordVoterLabel);
        searchVoterMainLayout->addSpacing(10);
        searchListWidgetCandidateLayout->addWidget(searchComboboxCandidate);
        searchListWidgetCandidateLayout->addSpacing(10);
        searchListWidgetCandidateLayout->addWidget(searchLineEditCandidate);
        searchListWidgetCandidateLayout->addSpacing(10);
        searchListWidgetCandidateLayout->addWidget(searchPushButtonCandidate);
        searchListWidgetCandidateLayout->addSpacing(5);
        searchListWidgetCandidateLayout->addWidget(refreshCandidateButton);
        searchListWidgetVoterLayout->addWidget(searchLineEditVoter);
        searchListWidgetVoterLayout->addSpacing(10);
        searchListWidgetVoterLayout->addWidget(searchPushButtonVoter);
        searchListWidgetVoterLayout->addSpacing(5);
        searchListWidgetVoterLayout->addWidget(refreshVoterButton);
        searchListWidgetCandidateLayout->setContentsMargins(10, 10, 10, 10);
        searchListWidgetVoterLayout->setContentsMargins(10, 10, 10, 10);
        searchKeywordCandidateLabel->hide();
        searchKeywordVoterLabel->hide();
        refreshCandidateButton->setVisible(false);
        refreshVoterButton->setVisible(false);
    }
    ~ListMemberWidget(){
        delete mainLayout;
    }

    void clearList() {
        for (auto it = set_list_theme_candidate.begin(); it != set_list_theme_candidate.end(); it++) {
            ListMemberWidgetItem* temp = nullptr;
            std::swap(*it, temp);
            delete temp;
        }
        set_list_theme_candidate.clear();
        for (auto it = set_list_item_candidate.begin(); it != set_list_item_candidate.end(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(*it, temp);
            delete temp;
        }
        set_list_item_candidate.clear();
        for (auto it = set_list_theme_voter.begin(); it != set_list_theme_voter.end(); it++) {
            ListMemberWidgetItem* temp = nullptr;
            std::swap(*it, temp);
            delete temp;
        }
        set_list_theme_voter.clear();
        for (auto it = set_list_item_voter.begin(); it != set_list_item_voter.end(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(*it, temp);
            delete temp;
        }
        set_list_item_voter.clear();

        candidateListWidget->clear();
        voterListWidget->clear();
    }

    void clearListCandidate() {
        for (auto it = set_list_theme_candidate.begin(); it != set_list_theme_candidate.end(); it++) {
            ListMemberWidgetItem* temp = nullptr;
            std::swap(*it, temp);
            delete temp;
        }
        set_list_theme_candidate.clear();
        for (auto it = set_list_item_candidate.begin(); it != set_list_item_candidate.end(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(*it, temp);
            delete temp;
        }
        set_list_item_candidate.clear();
        candidateListWidget->clear();
    }

    void clearListVoter() {
        for (auto it = set_list_theme_voter.begin(); it != set_list_theme_voter.end(); it++) {
            ListMemberWidgetItem* temp = nullptr;
            std::swap(*it, temp);
            delete temp;
        }
        set_list_theme_voter.clear();
        for (auto it = set_list_item_voter.begin(); it != set_list_item_voter.end(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(*it, temp);
            delete temp;
        }
        set_list_item_voter.clear();
        voterListWidget->clear();
    }
};

class resultItem : public QWidget {
    Q_OBJECT
public:
    QVBoxLayout* mainLayout;
    QVBoxLayout* namepkLayout;
    QLabel* nameLabel;
    QLabel* pkLabel;
    QVBoxLayout* voteLayout;
    QProgressBar* voteBar;
    QLabel* voteLabel;

    resultItem(QWidget* parent = nullptr) : QWidget(parent) {
        QFont namef("Segoe UI", 12, 600, false);
        QFont pkf("Segoe UI", 9, 500, true);
        QFont pfkf("Segoe UI", 10, 700, false);
        mainLayout = new QVBoxLayout();
        namepkLayout = new QVBoxLayout();
        nameLabel = new QLabel("Airlangga Susanto Putra");
        nameLabel->setFont(namef);
        pkLabel = new QLabel("Publickey : 1234567812345678123456781234567812345678123456781234567812345678");
        pkLabel->setFont(pkf);
        namepkLayout->addWidget(nameLabel);
        namepkLayout->addWidget(pkLabel);
        namepkLayout->setAlignment(Qt::AlignLeft);
        mainLayout->addLayout(namepkLayout);
        voteLayout = new QVBoxLayout;
        voteBar = new QProgressBar;
        voteBar->setMaximum(50);
        voteBar->setValue(23);
        voteBar->setStyleSheet("QProgressBar { border: none; text-align: center; background-color: transparent; color: black; outline:none; } \
                            QProgressBar::chunk { \
                                border-radius: 10px; \
                                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, \
                                                                   stop:0 #D6EAF8, stop:1 #85C1E9); \
                            } \
                            QProgressBar::sub-page { \
                                border-radius: 10px; \
                                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, \
                                                                   stop:0 #D6EAF8, stop:1 #85C1E9); \
                            } \
                            QProgressBar::chunk:disabled { \
                                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, \
                                                                   stop:0 #E8E8E8, stop:1 #D6D6D6); \
                            }");
        voteBar->setTextVisible(false);
        voteLabel = new QLabel("23 Votes / 50 Votes");
        voteLayout->addWidget(voteBar);
        voteLayout->addWidget(voteLabel);
        voteLabel->setAlignment(Qt::AlignLeft);
        voteLabel->setFont(pfkf);
        voteLayout->setAlignment(Qt::AlignLeft);
        voteBar->setAlignment(Qt::AlignLeft);
        mainLayout->addLayout(voteLayout);
        setLayout(mainLayout);
        setStyleSheet("background-color:none;");
    }


    ~resultItem() {

    }
};

class ResultWidget : public QWidget {
    Q_OBJECT
public:
    QVBoxLayout* mainLayout;
    QVBoxLayout* topLayout;
    QLabel* titleLabel;
    QLabel* moreLabel;
    QVBoxLayout* frameLayout;
    QFrame* frameListWidget;
    QListWidget* listWidget;
    QVector<resultItem*> listTheme;
    QVector<QListWidgetItem*> listItem;
    QString hvp;
    long long totalvote;
    ResultWidget(QWidget* parent = nullptr) : QWidget(parent) {
        totalvote = 0;
        QFont fontT("Segoe UI", 18, 800, false);
        QFont fontM("Segoe UI", 12, 600, false);
        mainLayout = new QVBoxLayout();
        topLayout = new QVBoxLayout;
        moreLabel = new QLabel("Test Polling has concluded with 50 votes from a total of 500 participants.");
        moreLabel->setAlignment(Qt::AlignCenter);
        moreLabel->setFont(fontM);
        titleLabel = new QLabel("Polling Result");
        titleLabel->setFont(fontT);
        titleLabel->setAlignment(Qt::AlignCenter);
        topLayout->addWidget(titleLabel);
        topLayout->addWidget(moreLabel);
        frameListWidget = new QFrame();
        listWidget = new QListWidget();
        mainLayout->addLayout(topLayout);
        frameLayout = new QVBoxLayout();
        frameLayout->addWidget(listWidget);
        frameListWidget->setLayout(frameLayout);
        mainLayout->addWidget(frameListWidget);
        listWidget->setStyleSheet("QListWidget{border: 2px solid #1DA1F2; outline:none;}");
        setLayout(mainLayout);
    }

    ~ResultWidget() {
        delete listWidget;
        delete titleLabel;
        delete frameLayout;
        delete frameListWidget;
        delete mainLayout;
    }

    void clearlistWidget() {
        for (int it = 0; it < listTheme.size(); it++) {
            resultItem* temp = nullptr;
            std::swap(listTheme[it], temp);
            delete temp;
        }
        listTheme.clear();
        for (int it = 0; it < listItem.size(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(listItem[it], temp);
            delete temp;
        }
        listItem.clear();
        listWidget->clear();
    }
};

class voteItem : public QWidget {
    Q_OBJECT
public:
    QHBoxLayout* mainLayout;
    QVBoxLayout* namepkLayout;
    QPushButton* sendButton;
    QLabel* nameLabel;
    QLabel* pkLabel;
    QString pk;
    QString hvp;
    voteItem(QWidget* parent = nullptr) : QWidget(parent) {
        QFont namef("Segoe UI", 12, 600, false);
        QFont pkf("Segoe UI", 9, 500, true);
        mainLayout = new QHBoxLayout(this);
        namepkLayout = new QVBoxLayout();
        sendButton = new QPushButton("Vote");
        nameLabel = new QLabel("Name :");
        nameLabel->setFont(namef);
        pkLabel = new QLabel("Publickey :");
        pkLabel->setFont(pkf);
        namepkLayout->addWidget(nameLabel);
        namepkLayout->addWidget(pkLabel);
        namepkLayout->setAlignment(Qt::AlignLeft);
        mainLayout->addLayout(namepkLayout);
        mainLayout->addStretch();
        mainLayout->addWidget(sendButton);
        setLayout(mainLayout);
        setStyleSheet("QPushButton {"
            "   background-color: #1DA1F2;"
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #0D95E8;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0B7BC0;"
            "}"
            "QLineEdit {"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "outline:none;"
            "}"
            "QPushButton:focus { outline: none; }");
        connect(sendButton, &QPushButton::clicked, [&]() {
            emit doVote(hvp, pk);
            });
    }

    ~voteItem() {

    }

signals:
    void doVote(QString, QString);
};

class VoteDialog : public QDialog {
public:
    QVBoxLayout* mainLayout;
    QLabel* titleLabel;
    QLabel* moreLabel;
    QListWidget* listWidget;
    QVector<voteItem*> listTheme;
    QVector<QListWidgetItem*> listItem;
    QString hvp;
    VoteDialog() {
        QFont fontT("Segoe UI", 18, 800, false);
        QFont fontM("Segoe UI", 12, 600, true);
        mainLayout = new QVBoxLayout;
        titleLabel = new QLabel("Ballot");
        titleLabel->setFont(fontT);
        titleLabel->setAlignment(Qt::AlignCenter);
        moreLabel = new QLabel("Choose a candidate that you will support");
        moreLabel->setFont(fontM);
        moreLabel->setAlignment(Qt::AlignCenter);
        listWidget = new QListWidget;
        
        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(moreLabel);
        mainLayout->addWidget(listWidget);

        setLayout(mainLayout);
        setFixedWidth(600);
        setFixedHeight(450);
        QString stylesheetButton = "QDialog{background-color: #FFFFFF;}"
            "QListWidget{border: 2px solid #1DA1F2; outline:none; }";
        setStyleSheet(stylesheetButton);
    }

    ~VoteDialog() {
        delete titleLabel;
        delete moreLabel;
        listWidget->clear();
        delete listWidget;
        delete mainLayout;
    }
    void clearlistWidget() {
        for (int it = 0; it < listTheme.size(); it++) {
            voteItem* temp = nullptr;
            std::swap(listTheme[it], temp);
            delete temp;
        }
        listTheme.clear();
        for (int it = 0; it < listItem.size(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(listItem[it], temp);
            delete temp;
        }
        listItem.clear();
        listWidget->clear();
    }
};

class PleaseWaitDialog : public QDialog {
public:
    PleaseWaitDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        setModal(true);
        QFont font("Segoe UI", 11, 600, false);
        QIcon icon(":/TRSVote/styles/image/dialogico.png");
        setFixedSize(200, 50);
        setWindowTitle("TRS Vote");
        setStyleSheet("background-color:none;");
        setWindowIcon(icon);
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Please wait...", this);
        label->setAlignment(Qt::AlignCenter);
        label->setFont(font);

        layout->addWidget(label);
    }
};

class PanelWidget : public QWidget {
    Q_OBJECT
public:
    QFrame* frameUsername;
    QVBoxLayout* usernameLayout;
    QLabel* usernameLabel;
    QLabel* usernameDataLabel;
    QLabel* roleLabel;
    QLabel* roleDataLabel;
    QLabel* titleLabel;
    QFrame* sideBar;
    LogoutButton* logoutButton;
    KeypairButton* keypairButton;
    //ProfileButton* profileButton;
    BackButton* backButton;
    HistoryButton* historyButton;
    HistoryWidget* historyWidget;
    QVBoxLayout* sideBarTop;
    QVBoxLayout* sideBarBottom;

    QHBoxLayout* buttonBarLayout;
    QPushButton* button1;
    QPushButton* button2;
    QPushButton* button3;
    QPushButton* button4;

    QHBoxLayout* footerLayout;
    QPushButton* greenButton;
    QLabel* greenLabel;
    QPushButton* yellowButton;
    QLabel* yellowLabel;
    QPushButton* redButton;
    QLabel* redLabel;

    QVBoxLayout* sideBarLayout;
    QGridLayout* page1GridLayout;
    QGridLayout* centralLayout;
    QWidget* centralWidget;
    ListMemberWidget* listMemberWidget;
    QStackedWidget* contentStack;
    QFrame* contentPage1;
    QListWidget* listPoll;
    std::unordered_map<std::string, QListWidgetItem*> set_list_item;
    std::unordered_map<std::string, ClickableLayoutWidget*> set_list_theme;
    QVector<std::string> set_list_hvp;
    int method;
    MailWidget* mailWidget;
    int backflag;
    CreateVotepollWidget* createVotepollWidget;
    ResultWidget* resultWidget;
    PanelWidget(QWidget* parent = nullptr) : QWidget(parent) {
        listMemberWidget = new ListMemberWidget;
        QFont bold("Segoe UI", 10, 800, false);
        frameUsername = new QFrame;
        frameUsername->setFrameShape(QFrame::Box);
        frameUsername->setFrameShadow(QFrame::Sunken);
        frameUsername->setLineWidth(1);
        usernameLayout = new QVBoxLayout;
        usernameLabel = new QLabel("Username :");
        usernameLayout->addWidget(usernameLabel);
        usernameDataLabel = new QLabel("AirlanggaSusantoPutra");
        usernameLayout->addWidget(usernameDataLabel);
        roleLabel = new QLabel("Role :");
        usernameLayout->addWidget(roleLabel);
        roleDataLabel = new QLabel("Admin");
        usernameLayout->addWidget(roleDataLabel);
        frameUsername->setLayout(usernameLayout);
        footerLayout = new QHBoxLayout;
        greenButton = new QPushButton;
        greenLabel = new QLabel("elections are in progress.");
        redButton = new QPushButton;
        redLabel = new QLabel("elections have concluded.");
        yellowButton = new QPushButton;
        yellowLabel = new QLabel("election registration is open.");
        greenButton->setStyleSheet("border:none;""outline:none;""background-color:#00FF00");
        greenButton->setFixedWidth(15);
        redButton->setStyleSheet("border:none;""outline:none;""background-color:#FF0000");
        redButton->setFixedWidth(15);
        yellowButton->setStyleSheet("border:none;""outline:none;""background-color:#FFFF00");
        yellowButton->setFixedWidth(15);
        greenButton->setDisabled(true);
        redButton->setDisabled(true);
        yellowButton->setDisabled(true);
        footerLayout->addWidget(greenButton);
        footerLayout->addWidget(greenLabel);
        footerLayout->addWidget(yellowButton);
        footerLayout->addWidget(yellowLabel);
        footerLayout->addWidget(redButton);
        footerLayout->addWidget(redLabel);
        footerLayout->setAlignment(Qt::AlignBottom);
        createVotepollWidget = new CreateVotepollWidget;
        historyWidget = new HistoryWidget;
        resultWidget = new ResultWidget;
        resultWidget->hide();
        backflag = 0;
        mailWidget = new MailWidget;
        backButton = new BackButton;
        method = 0;
        titleLabel = new QLabel("Polling Station");
        titleLabel->setAlignment(Qt::AlignCenter);
        QFont titleFont("Segoe UI", 18, 800, false);
        QFont fontall("Segoe UI", 10, 600, false);
        QString stylesheetButton = "QPushButton {"
            "   background-color: #1DA1F2;"
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #0D95E8;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #E1E8ED;"
            "   color: #AAB8C2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0B7BC0;"
            "}"
            "QLineEdit {"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "outline:none;"
            "}"
            "QPushButton:focus { outline: none; }";
        titleLabel->setFont(titleFont);
        sideBar = new QFrame;
        logoutButton = new LogoutButton;
        keypairButton = new KeypairButton;
        //profileButton = new ProfileButton;
        historyButton = new HistoryButton;
        sideBarTop = new QVBoxLayout;
        sideBarTop->addLayout(backButton);
        sideBarTop->addWidget(frameUsername);
        sideBarBottom = new QVBoxLayout;
        sideBarBottom->addLayout(historyButton);
        //sideBarBottom->addLayout(profileButton);
        sideBarBottom->addSpacing(5);
        sideBarBottom->addLayout(keypairButton);
        sideBarBottom->addSpacing(5);
        sideBarBottom->addLayout(logoutButton);

        contentStack = new QStackedWidget;
        contentPage1 = new QFrame;
        listPoll = new QListWidget;

        buttonBarLayout = new QHBoxLayout;
        button1 = new QPushButton("Create Polling");
        button2 = new QPushButton("List Member");
        button3 = new QPushButton("Add Member");
        button4 = new QPushButton("Check Detail");
        button1->setStyleSheet(stylesheetButton);
        button2->setStyleSheet(stylesheetButton);
        button3->setStyleSheet(stylesheetButton);
        button4->setStyleSheet(stylesheetButton);
        button1->setFont(fontall);
        button2->setFont(fontall);
        button3->setFont(fontall);
        button4->setFont(fontall);
        buttonBarLayout->addWidget(button1);
        buttonBarLayout->addWidget(button3);
        buttonBarLayout->addWidget(button2);
        buttonBarLayout->addWidget(button4);
        buttonBarLayout->addStretch();

        centralWidget = new QWidget;

        sideBarLayout = new QVBoxLayout;

        page1GridLayout = new QGridLayout;
        centralLayout = new QGridLayout;

        sideBarLayout->addLayout(sideBarTop);
        sideBarLayout->addStretch();
        sideBarLayout->addLayout(sideBarBottom);
        sideBar->setLayout(sideBarLayout);
        sideBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sideBar->setFixedWidth(120);  // Increase the width of the sideBar

        page1GridLayout->addWidget(titleLabel, 0, 0, 1, 2);
        page1GridLayout->addLayout(buttonBarLayout, 1, 0, 1, 2);  // Add button bar at the top
        page1GridLayout->addWidget(listPoll, 2, 0, 1, 2);
        page1GridLayout->addLayout(footerLayout, 3, 0, 1, 2);


        contentPage1->setLayout(page1GridLayout);
        contentStack->addWidget(listMemberWidget);
        contentStack->addWidget(createVotepollWidget);
        contentStack->addWidget(contentPage1);
        contentStack->addWidget(mailWidget);
        contentStack->addWidget(resultWidget);
        contentStack->addWidget(historyWidget);
        contentStack->setCurrentWidget(contentPage1);
        contentStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        /* Finally, set up the main elements into the central layout... */
        centralLayout->addWidget(sideBar, 1, 0, 1, 1);
        centralLayout->addWidget(contentStack, 1, 1, 1, 1);
        centralWidget->setLayout(centralLayout);
        setLayout(centralLayout);
        setStyleSheet("QLineEdit {"
            "   background-color: #E1E8ED;"
            "   border: none;"
            "   border-radius: 4px;"
            "   padding: 6px;"
            "}"
            "QPushButton:focus { outline: none; }"
            "QScrollBar:vertical {"
            "    background-color: #E1E8ED;"
            "    width: 10px;"
            "    margin: 0px;"
            "}"
            "QScrollBar::handle:vertical {"
            "    background-color: #79ADF9;"
            "    min-height: 20px;"
            "}"
            "QScrollBar::sub-page:vertical, QScrollBar::add-page:vertical {"
            "    background-color: none;"
            "}"
        );
        listPoll->setStyleSheet("QListWidget{border: 2px solid #1DA1F2;}");
        button2->setDisabled(true);
        button4->setDisabled(true);
        button3->setDisabled(true);
        backButton->logoutButton->hide();
        backButton->logoutLabel->hide();
        usernameLabel->setFont(bold);
        roleLabel->setFont(bold);
        connect(button1, &QPushButton::clicked, [&]() {
            QDateTime minimumDateTimeStart = QDateTime::currentDateTime().addSecs(1800);
        createVotepollWidget->dateTimeEditStart->setMinimumDateTime(minimumDateTimeStart);

        QDateTime minimumDateTimeEnd = minimumDateTimeStart.addSecs(3600);
        createVotepollWidget->dateTimeEditEnd->setMinimumDateTime(minimumDateTimeEnd);
            backflag = 1;
            backButton->showButton();
            frameUsername->hide();
            contentStack->setCurrentWidget(createVotepollWidget);
            });

        connect(backButton->logoutButton, &QPushButton::clicked, [&]() {
            backButton->hideButton();
            if (backflag == 2) {
                //profileButton->showButton();
            }
            if (backflag == 3) {
                historyButton->showButton();
            }
            frameUsername->show();
            contentStack->setCurrentWidget(contentPage1);
            
            });

        //connect(profileButton->logoutButton, &QPushButton::clicked, [&]() {
        //    backflag = 2;
        //    profileButton->hideButton();
        //    frameUsername->hide();
        //    contentStack->setCurrentWidget(mailWidget);
        //    backButton->showButton();

        //    });
        connect(historyButton->logoutButton, &QPushButton::clicked, [&]() {
            backflag = 3;
            historyButton->hideButton();
            frameUsername->hide();
            contentStack->setCurrentWidget(historyWidget);
            backButton->showButton();
            });

        connect(button2, &QPushButton::clicked, [&]() {
            int index = listPoll->currentRow();
        if (index < 0) {
            return;
        }
        else if (index >= set_list_hvp.size()) {
            return;
        }
        QString hvp = set_list_hvp[index].c_str();
        emit dialogHVP(hvp, 1);
            });
        connect(button3, &QPushButton::clicked, [&]() {
            int index = listPoll->currentRow();
        if (index < 0) {
            return;
        }
        else if (index >= set_list_hvp.size()) {
            return;
        }
        QString hvp = set_list_hvp[index].c_str();
        emit dialogHVP(hvp, 2);
            });
        connect(button4, &QPushButton::clicked, [&]() {
            int index = listPoll->currentRow();
        if (index < 0) {
            return;
        }
        else if (index >= set_list_hvp.size()) {
            return;
        }
        QString hvp = set_list_hvp[index].c_str();
        emit dialogHVP(hvp, 3);
            });
    }

    void setupPanel(int m) {
        button2->setDisabled(true);
        button3->setDisabled(true);
        button4->setDisabled(true);
        backButton->hideButton();
        frameUsername->show();
        contentStack->setCurrentWidget(contentPage1);
        method = m;
        if (method == 1) {
            button1->show();
            button2->show();
            button3->show();
            //profileButton->showButton();
            historyButton->hideButton();
        }
        else if (method == 2) {
            button1->hide();
            button2->hide();
            button3->hide();
            //profileButton->hideButton();
            historyButton->showButton();
        }
        //for (auto it = set_list_theme.begin(); it != set_list_theme.end(); it++) {
        //    if (method == 1) {
        //        it->second->setLabelColor(1);
        //    }
        //    else if (method == 2) {
        //        it->second->setLabelColor(2);
        //    }
        //}
    }

    void updateDatePolling() {
        if (set_list_theme.empty()) {
            return;
        }
        for (auto it = set_list_theme.begin(); it != set_list_theme.end(); it++) {
            it->second->setLabelColor(method);
        }
    }

    void logoutPanel() {
        set_list_hvp.clear();
        for (auto it = set_list_theme.begin(); it != set_list_theme.end(); it++) {
            ClickableLayoutWidget* temp = nullptr;
            std::swap(it->second, temp);
            delete temp;
        }
        set_list_theme.clear();
        for (auto it = set_list_item.begin(); it != set_list_item.end(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(it->second, temp);
            delete temp;
        }
        set_list_item.clear();
        listPoll->clear();
        mailWidget->clearMail();
        historyWidget->logoutUser();
        method = 0;
    }

signals:
    void dialogHVP(QString hvp, int m);
};