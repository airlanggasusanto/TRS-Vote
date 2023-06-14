#include "TRSVote.h"
#include <QDebug>
#include <QTimer>
#include <QDesktopServices>
#include <QItemSelection>
#include <QWindow>
#include <QProcess>
#include <QFile>
#include <QTextStream>

TRSVote::TRSVote(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    stackedWidget = new QStackedWidget;
    genesisVotepoll = nullptr;
    storedKey = nullptr;
    setCentralWidget(stackedWidget);
    SetupUI();
    SetupLoginSystem();
    SetupPanelSystem();
    SetupMempool();
    SetupPleaseWait();
    connect(ui.actionClose, &QAction::triggered, [&]() {
        QProcess::startDetached(QApplication::applicationFilePath());
        QCoreApplication::quit();
        });

    connect(ui.actionUser_Manual, &QAction::triggered, [&]() {
        QString url = "https://github.com/airlanggasusanto/TRS-Vote/blob/main/doc/UserManual.pdf";
        QDesktopServices::openUrl(QUrl(url));
        });

    aboutTRSVote = new dialogAbout;
    connect(ui.actionAbout_TRS_Vote, &QAction::triggered, [&]() {
        aboutTRSVote->exec();
        });

    nsdialog->tryloop();
}

TRSVote::~TRSVote()
{
    _mempool->abort();
    _mempoolthread->wait();
    if (_mempool != nullptr) {
        delete _mempool;
        delete _mempoolthread;
        _mempool = nullptr;
        _mempoolthread = nullptr;
    }
    _helper->abort();
    _helperthread->wait();
    if (_helper != nullptr) {
        delete _helper;
        delete _helperthread;
        _helper = nullptr;
        _helperthread = nullptr;
    }
    delete blockheightlabel;
    delete peerlabel;
    delete loadingBar;
    delete statusloadingBar;
    delete stackedWidget;
    delete keypairDialog;
    delete addDialog;
    delete detailDialog;
    delete registerDialog;
    for (auto it = set_display.begin(); it != set_display.end(); it++) {
        VoteTree::display* tmp = nullptr;
        std::swap(tmp, it->second);
        delete tmp;
    }
    set_display.clear();
}

void TRSVote::SetupUI() {
    QIcon iconui(":/TRSVote/styles/image/TRS Vote Logo.png");
    this->setWindowIcon(iconui);
    QCoreApplication::setOrganizationName("Student - Universitas Pembangunan Nasional Jawa Timur");
    QString statusbarStyle = "QStatusBar {"
        "background-color: #FFFFFF;"
        "border: none;"
        "color: #14171A;"
        "}"
        "QStatusBar::item {"
        "border: none;"
        "background-color: #FFFFFF;"
        "color: #14171A;"
        "border-radius: 2px;"
        "}"
        "QStatusBar::item:selected {"
        "background-color: #1DA1F2;"
        "color: #FFFFFF;"
        "}";
    ui.statusBar->setStyleSheet(statusbarStyle);
    QString menubarStyle = "QMenuBar {"
        "background-color: #FFFFFF;"
        "border: none;"
        "spacing: 10px;"
        "}"
        "QMenuBar::item {"
        "spacing: 3px;"
        "padding: 1px 4px;"
        "background-color: #FFFFFF;"
        "color: #14171A;"
        "border-radius: 2px;"
        "}"
        "QMenuBar::item:selected {"
        "background-color: #1DA1F2;"
        "color: #FFFFFF;"
        "}"
        "QMenu {"
        "border: none;"
        "background-color: #FFFFFF;"
        "margin: 2px 0px 2px 0px;"
        "}"
        "QMenu::item {"
        "padding: 3px 20px;"
        "border: none;"
        "background-color: transparent;"
        "color: #14171A;"
        "}"
        "QMenu::item:selected {"
        "background-color: #1DA1F2;"
        "color: #FFFFFF;"
        "}"
        "QMenu::separator {"
        "height: 1px;"
        "background-color: #E1E8ED;"
        "margin: 4px 0px 4px 0px;"
        "}";
    ui.menuBar->setStyleSheet(menubarStyle);
    this->setStyleSheet("QWidget {"
        "background-color: #FFFFFF;"
        "}");
    this->setMinimumHeight(500);
    this->setMinimumWidth(850);
    // Create a new label widget
    blockheightlabel = new QLabel("Block Height : - ", this);
    blockheightlabel->setFixedWidth(150);
    blockheightlabel->setStyleSheet("color: #14171A; font-family: Segoe UI; font-size: 12px; font-weight: bold; font-weight: 500; padding-left: 4px;");
    // Add the label to the status bar, on the left
    peerlabel = new QLabel("Peer : No Connection ", this);
    peerlabel->setFixedWidth(150);
    peerlabel->setStyleSheet("color: #14171A; font-family: Segoe UI; font-size: 12px; font-weight: bold; font-weight: 500; padding-left: 4px;");
    ui.statusBar->addWidget(blockheightlabel);
    ui.statusBar->addWidget(peerlabel);

    loadingBar = new QProgressBar(this);
    statusloadingBar = new QLabel("Validating Block", this);
    statusloadingBar->setAlignment(Qt::AlignRight);
    loadingBar->setAlignment(Qt::AlignLeft);
    loadingBar->setFixedWidth(200);
    // Add the progress bar and label to the status bar
    ui.statusBar->addPermanentWidget(loadingBar);
    ui.statusBar->addPermanentWidget(statusloadingBar);

    // Set the progress bar properties
    loadingBar->setRange(0, 0);
    loadingBar->setTextVisible(false);
    // Set the stylesheet for the progress bar
    loadingBar->setStyleSheet("QProgressBar {"
        "border: 2px solid #1DA1F2;"
        "border-radius: 10px;"
        "background-color: #FFFFFF;"
        "height: 12px;"
        "}"
        "QProgressBar::chunk {"
        "border-radius: 10px;"
        "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #1DA1F2, stop: 1 #1665D8);"
        "}");

    // Set the stylesheet for the loading label
    statusloadingBar->setStyleSheet("color: #14171A;"
        "font-family: Segoe UI;"
        "font-size: 12px;"
        "font-weight: bold;"
        "font-weight: 500;");
    loadingBar->show();
    statusloadingBar->hide();
}

void TRSVote::SetupPleaseWait() {
    pleaseWaitWidget = new QWidget;
    QFont font("Segoe UI", 60, 800, false);
    QLabel* labelPleaseWait = new QLabel("TRS Vote");
    QLabel* labelBottom = new QLabel("Securing Election System.");
    QFont fontbottom("Segoe UI", 21, 600, true);
    labelBottom->setFont(fontbottom);
    labelBottom->setAlignment(Qt::AlignCenter);
    labelPleaseWait->setStyleSheet("color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #1DA1F2, stop: 1 #1665D8);");
    labelPleaseWait->setAlignment(Qt::AlignCenter);
    labelPleaseWait->setFont(font);
    QVBoxLayout* Layout = new QVBoxLayout;
    Layout->addStretch(2);
    Layout->addWidget(labelPleaseWait);
    Layout->addWidget(labelBottom);
    Layout->addStretch(2);
    Layout->setAlignment(Qt::AlignCenter);
    pleaseWaitWidget->setLayout(Layout);
    stackedWidget->addWidget(pleaseWaitWidget);
    if (genesisVotepoll == nullptr) {
        stackedWidget->setCurrentWidget(pleaseWaitWidget);
    }
}

void TRSVote::SetupLoginSystem() {
    loginWidget = new LoginForm;
    registerWidget = new LoginForm;
    registerWidget->titleLabel->setText("Setup Election Administrator");
    stackedWidget->addWidget(loginWidget);
    stackedWidget->addWidget(registerWidget);
    connect(registerWidget->loginButton, &QPushButton::clicked, [&]() {
        QString t_u = registerWidget->usernameLineEdit->text();
        QString t_p = registerWidget->passwordLineEdit->text();
        if (!checkThatAccountValid(t_u, t_p)) {
            return;
        }
        if (storedKey != nullptr) {
            delete storedKey;
            storedKey = nullptr;
        }
        storedKey = new QKeypair;
        storedKey->username = t_u.toStdString();
        storedKey->password = t_p.toStdString();
        storedKey->calculate_keypair();
        Block::VotePoll vp;
        memcpy(vp.publickey, storedKey->key.publickey, 32);
        Block::calculate_votepoll_signature(&vp, vp.signature, &storedKey->key);
        Block::Data bd;
        Block::votepoll_to_blockdata(&vp, bd);
        int size = static_cast<int>(bd.datsize) + 4;
        QByteArray qb = QByteArray((char*)&bd.data[0], size);
        _mempool->receiveNewData(&qb);
        registerWidget->makeWait();
        storedKey->role = 1;
        loadingBar->show();
        });
    connect(loginWidget->loginButton, &QPushButton::clicked, [&]() {
        QKeypair temp;
        temp.username = loginWidget->usernameLineEdit->text().toStdString();
        temp.password= loginWidget->passwordLineEdit->text().toStdString();
        if (!checkThatAccountValid(temp.username.c_str(), temp.password.c_str())) {
            return;
        }
        temp.calculate_keypair();
        if (storedKey != nullptr) {
            delete storedKey;
            storedKey = nullptr;
        }
        storedKey = new QKeypair(temp);
        QByteArray privatekey = QByteArray((char*)&temp.key.privatekey[0], 32);

        if (genesisVotepoll != nullptr) {
            if (sodium_memcmp(storedKey->key.publickey, genesisVotepoll->publickey, 32) == 0) {
                storedKey->role = 1;
            }
            else {
                storedKey->role = 2;
            }
            loginWidget->makeWait();
            _helper->Method_User(0, &privatekey);
        }
        else {
            Information("We are sorry, but we are currently unable to process your login request. Please try again later");
        }

        //if (genesisVotepoll != nullptr) {
        //    loginWidget->usernameLineEdit->clear();
        //    loginWidget->passwordLineEdit->clear();
        //    if (sodium_memcmp(genesisVotepoll->publickey, storedKey->key.publickey, 32) == 0) {
        //        storedKey->role = 1;
        //        panelWidget->usernameDataLabel->setText(storedKey->username.c_str());
        //        panelWidget->roleDataLabel->setText("Admin");
        //        panelWidget->setupPanel(1);
        //        for (auto open = panelWidget->mailWidget->set_list_theme.begin(); open != panelWidget->mailWidget->set_list_theme.end(); open++) {
        //            open->second->openCipher(storedKey->key.privatekey);
        //        }
        //        stackedWidget->setCurrentWidget(panelWidget);
        //    }
        //    else {
        //        panelWidget->usernameDataLabel->setText(storedKey->username.c_str());
        //        panelWidget->roleDataLabel->setText("User");
        //        std::string pks = Utility::uint8_t_to_hexstring(storedKey->key.publickey, 32);
        //        for (auto it = set_display.begin(); it != set_display.end(); it++) {
        //            auto findclick = panelWidget->set_list_theme.find(it->first);
        //            std::string hvppk = it->first + pks;
        //            if (auto findmsg = panelWidget->mailWidget->set_list_theme.find(hvppk); findmsg != panelWidget->mailWidget->set_list_theme.end()) {
        //                findmsg->second->openCipherAsUser(&storedKey->key, genesisVotepoll->publickey);
        //                HistoryItem* theme = new HistoryItem;
        //                theme->dateLabel->setText(QDateTime::fromSecsSinceEpoch(findmsg->second->blocktime).toString("h:mm AP\nddd, M/d/yyyy"));
        //                if (findmsg->second->irole == 1) {
        //                    theme->detailLabel->setText("Requested as candidate at " + findmsg->second->desc);
        //                }
        //                else {
        //                    theme->detailLabel->setText("Requested as voter at " + findmsg->second->desc);
        //                }
        //                QListWidgetItem* item = new QListWidgetItem;
        //                item->setSizeHint(theme->sizeHint());
        //                panelWidget->historyWidget->listWidget->insertItem(0, item);
        //                panelWidget->historyWidget->listWidget->setItemWidget(item, theme);
        //                panelWidget->historyWidget->set_list_theme_req.emplace(it->first, theme);
        //                panelWidget->historyWidget->set_list_item_req.emplace(it->first, item);
        //                if (findclick != panelWidget->set_list_theme.end()) {
        //                    findclick->second->requestFlag = true;
        //                }
        //            }
        //            if (auto finddc = it->second->dc_index.find(pks); finddc != it->second->dc_index.end()) {
        //                int index = finddc->second;
        //                HistoryItem* theme = new HistoryItem;
        //                theme->detailLabel->setText("Registered as candidate at " + it->second->vp.desc);
        //                theme->dateLabel->setText(QDateTime::fromSecsSinceEpoch(it->second->dc[index].blocktime).toString("h:mm AP\nddd, M/d/yyyy"));
        //                QListWidgetItem* item = new QListWidgetItem;
        //                item->setSizeHint(theme->sizeHint());
        //                panelWidget->historyWidget->listWidget->insertItem(0,item);
        //                panelWidget->historyWidget->listWidget->setItemWidget(item, theme);
        //                panelWidget->historyWidget->set_list_theme_reg.emplace(it->first, theme);
        //                panelWidget->historyWidget->set_list_item_reg.emplace(it->first, item);
        //                if (findclick != panelWidget->set_list_theme.end()) {
        //                    findclick->second->registeredFlag = true;
        //                    findclick->second->asCandidateFlag = true;
        //                }
        //            }
        //            if (auto finddp = it->second->dp_index.find(pks); finddp != it->second->dp_index.end()) {
        //                int index = finddp->second;
        //                HistoryItem* theme = new HistoryItem;
        //                theme->detailLabel->setText("Registered as voter at " + it->second->vp.desc);
        //                theme->dateLabel->setText(QDateTime::fromSecsSinceEpoch(it->second->dp[index].blocktime).toString("h:mm AP\nddd, M/d/yyyy"));
        //                QListWidgetItem* item = new QListWidgetItem;
        //                item->setSizeHint(theme->sizeHint());
        //                panelWidget->historyWidget->listWidget->insertItem(0,item);
        //                panelWidget->historyWidget->listWidget->setItemWidget(item, theme);
        //                panelWidget->historyWidget->set_list_theme_reg.emplace(it->first, theme);
        //                panelWidget->historyWidget->set_list_item_reg.emplace(it->first, item);
        //                if (findclick != panelWidget->set_list_theme.end()) {
        //                    findclick->second->registeredFlag = true;
        //                    findclick->second->nonVFlag = false;
        //                }
        //            }
        //            else {
        //                findclick->second->nonVFlag = true;
        //            }
        //            TRS::Tag mytag;
        //            if (it->second->get_tag(pks, mytag)) {
        //                std::string hashtag = VoteTree::get_hash_hex_tag(&mytag);
        //                if (auto finit = it->second->vote_index.find(hashtag); finit != it->second->vote_index.end()) {
        //                    VoteTree::VoteString* vs = &it->second->vote[finit->second];
        //                    for (int a = 0; a < it->second->dc.size(); a++) {
        //                        Block::DataVote dv;
        //                        Utility::hexstring_to_uint8_t(dv.hashdatacandidate, 32, it->second->dc[a].hash.toStdString());
        //                        dv.tag = mytag;
        //                        Block::calculate_datavote_signature(&dv, dv.signature, &storedKey->key);
        //                        std::string a1 = Utility::uint8_t_to_hexstring(dv.signature.A_1, 32);
        //                        if (auto findvs = vs->vote_a1.find(a1); findvs != vs->vote_a1.end()) {
        //                            HistoryItem* theme = new HistoryItem;
        //                            theme->detailLabel->setText("Vote for " + it->second->dc[a].name + " in " + it->second->vp.desc);
        //                            theme->dateLabel->setText(QDateTime::fromSecsSinceEpoch(vs->block_time.find(a1)->second).toString("h:mm AP\nddd, M/d/yyyy"));
        //                            QListWidgetItem* item = new QListWidgetItem;
        //                            item->setSizeHint(theme->sizeHint());
        //                            panelWidget->historyWidget->listWidget->insertItem(0, item);
        //                            panelWidget->historyWidget->listWidget->setItemWidget(item, theme);
        //                            panelWidget->historyWidget->set_list_theme_vote.emplace(it->first, theme);
        //                            panelWidget->historyWidget->set_list_item_vote.emplace(it->first, item);
        //                            if (findclick != panelWidget->set_list_theme.end()) {
        //                                findclick->second->voteFlag = true;
        //                            }
        //                            break;
        //                        }
        //                    }
        //                }
        //            }
        //        }
        //        storedKey->role = 2;
        //        panelWidget->setupPanel(2);
        //        stackedWidget->setCurrentWidget(panelWidget);
        //    }
        //}
        //else {
        //    Information("We are sorry, but we are currently unable to process your login request. Please try again later");
        //}
        });
}

void TRSVote::SetupPanelSystem() {
    panelWidget = new PanelWidget;
    stackedWidget->addWidget(panelWidget);
    connect(panelWidget->logoutButton->logoutButton, &QPushButton::clicked, [&]() {
        QByteArray temp;
    _helper->Method_User(1, &temp);
    if (storedKey != nullptr) {
        delete storedKey;
        panelWidget->logoutPanel();
    }
    storedKey = nullptr;
    panelWidget->logoutPanel();
    stackedWidget->setCurrentWidget(loginWidget);
        });
    connect(panelWidget->createVotepollWidget->pushButtonSend, &QPushButton::clicked, [&]() {
        if (storedKey == nullptr) {
            Information("There was an error while creating polling.");
            return;
        }
    if (sodium_memcmp(storedKey->key.publickey, genesisVotepoll->publickey, 32) != 0) {
        Information("There was an error while creating polling.");
        return;
    }
    QDateTime minimumDateTimeStart = QDateTime::currentDateTime().addSecs(1800);
    panelWidget->createVotepollWidget->dateTimeEditStart->setMinimumDateTime(minimumDateTimeStart);

    QDateTime minimumDateTimeEnd = minimumDateTimeStart.addSecs(3600);
    panelWidget->createVotepollWidget->dateTimeEditEnd->setMinimumDateTime(minimumDateTimeEnd);

    QString desc = panelWidget->createVotepollWidget->lineEditDesc->text();
    if (desc.isEmpty()) {
        Information("The description of the polling is empty. Please provide a description.");
        return;
    }

    if (desc.length() > 64) {
        Information("The description length should not exceed 64 characters.");
        return;
    }
    long long currentdate = QDateTime::currentSecsSinceEpoch();
    long long startdate = panelWidget->createVotepollWidget->dateTimeEditStart->dateTime().toSecsSinceEpoch();
    long long enddate = panelWidget->createVotepollWidget->dateTimeEditEnd->dateTime().toSecsSinceEpoch();

    if (startdate >= enddate) {
        Information("The start date must be earlier than the end date.");
        return;
    }

    if (startdate < currentdate) {
        Information("The start date is earlier than the current date.");
        return;
    }

    if (currentdate > enddate) {
        Information("The current date is later than the end date.");
        return;
    }

    int ringsize = panelWidget->createVotepollWidget->spinBoxRingSize->value();
    int candidatesize = panelWidget->createVotepollWidget->spinBoxSizeCandidate->value();
    int participantsize = panelWidget->createVotepollWidget->spinBoxSizeParticipant->value();
    panelWidget->createVotepollWidget->lineEditDesc->clear();
    Block::VotePoll temp;
    memcpy(temp.publickey, storedKey->key.publickey, 32);
    Utility::ascii_to_uint8_t(desc.toStdString(), temp.description, 64);
    Utility::int_to_uint8_t(startdate, temp.startdate, 4);
    Utility::int_to_uint8_t(enddate, temp.enddate, 4);
    Utility::int_to_uint8_t(ringsize, temp.ringsize, 2);
    Utility::int_to_uint8_t(candidatesize, temp.candidatesize, 4);
    Utility::int_to_uint8_t(participantsize, temp.partisipansize, 4);
    Block::calculate_votepoll_signature(&temp, temp.signature, &storedKey->key);
    if (!Block::check_signature_votepoll(&temp)) {
        Information("Failed to create the polling.Please try again later.");
        return;
    }
    uint8_t hash[32]{};
    Block::get_hash_votepoll(&temp, hash);
    VoteTree::display s(&temp);
    s.vp.blocktime = 0;
    uint8_t vptype[4]{};
    uint8_t vpraw[182]{};
    Block::get_blockdata_type(1, vptype);
    memcpy(vpraw, vptype, 4);
    memcpy(vpraw + 4, &temp, sizeof(Block::VotePoll));
    QByteArray vpQ = QByteArray((char*)&vpraw[0], 182);
    ClickableLayoutWidget* newlayout = new ClickableLayoutWidget;
    newlayout->hashvotepoll = s.vp.hash;
    newlayout->pollLabel->setText(s.vp.desc);
    newlayout->startdate = s.vp.startdate;
    newlayout->enddate = s.vp.enddate;
    newlayout->maxcandidate = 0;
    newlayout->maxvoter = 0;
    newlayout->currentcandidate = 0;
    newlayout->currentvoter = 0;
    newlayout->blocktime = QDateTime::currentSecsSinceEpoch();
    newlayout->ringsize = s.vp.ringsize;
    newlayout->pk = s.vp.pk;
    newlayout->sig = s.vp.sig;
    if (storedKey != nullptr) {
        newlayout->setLabelColor(storedKey->role);
    }
    std::string key = s.vp.hash.toStdString();
    QListWidgetItem* newlist = new QListWidgetItem;
    newlist->setSizeHint(newlayout->sizeHint());
    panelWidget->listPoll->insertItem(0, newlist);
    panelWidget->listPoll->setItemWidget(newlist, newlayout);
    panelWidget->set_list_theme.emplace(key, newlayout);
    panelWidget->set_list_item.emplace(key, newlist);
    panelWidget->set_list_hvp.push_front(key);
    connect(newlayout, &ClickableLayoutWidget::registerHVP, this, &TRSVote::handleRegisterWHelper);
    connect(newlayout, &ClickableLayoutWidget::voteHVP, this, &TRSVote::handleVoteWHelper);
    connect(newlayout, &ClickableLayoutWidget::resultHVP, this, &TRSVote::handleResultWHelper);
    connect(newlayout, &ClickableLayoutWidget::mailHVP, this, &TRSVote::handleMailWHelper);
    newlayout->flagLabel->setText("Unconfirmed Transaction");
    newlayout->flagLabel->show();
    newlayout->mailButton->setVisible(false);
    _mempool->receiveNewData(&vpQ);
    Information("Create Polling Success");
        });

    QIcon dialogIcon(":/TRSVote/styles/image/dialogico.png");
    keypairDialog = new KeyPairDialog;
    keypairDialog->setWindowIcon(dialogIcon);
    connect(panelWidget->keypairButton->logoutButton, &QPushButton::clicked, [&]() {
        keypairDialog->privateLineEdit->lineEdit->setEchoMode(QLineEdit::Password);
    QString pub;
    QString priv;

    if (storedKey == nullptr) {
        Information(QString("Failed to getting keypair, Please try relogin, or try again later."));
        stackedWidget->setCurrentWidget(loginWidget);
        return;
    }
    pub = Utility::uint8_t_to_hexstring(storedKey->key.publickey, 32).c_str();
    priv = Utility::uint8_t_to_hexstring(storedKey->key.privatekey, 32).c_str();
    keypairDialog->publicLineEdit->setText(pub);
    keypairDialog->privateLineEdit->setText(priv);
    keypairDialog->exec();
        });

    registerDialog = new RegistrationForm;
    registerDialog->setWindowIcon(dialogIcon);
    connect(registerDialog->sendButton, &QPushButton::clicked, [&]() {
        QString name = registerDialog->nameLineEdit->text();
        QString idcard = registerDialog->idCardLineEdit->text();
        QString affiliate = registerDialog->affiliateLineEdit->text();
        int role = registerDialog->roleComboBox->currentIndex();
        QString info = registerDialog->descLineEdit->toPlainText();
        if (name.isEmpty()) {
            Information("Please provide your name.");
            return;
        }
        if (idcard.isEmpty()) {
            Information("Please enter your ID card information.");
            return;
        }
        if (affiliate.isEmpty()) {
            Information("Please provide your affiliate details.");
            return;
        }
        if (role == 0) {
            Information("Please choose a role.");
            return;
        }
        if (info.isEmpty()) {
            Information("Please provide the necessary information.");
            return;
        }
        if (storedKey == nullptr || registerDialog->hvp.isEmpty() || registerDialog->hvp.length() != 64) {
            Information("An error occurred while attempting to create this data. Please consider re-logging or try again later.");
            return;
        }
        auto finddisplay = panelWidget->set_list_theme.find(registerDialog->hvp.toStdString());
        if (finddisplay == panelWidget->set_list_theme.end()) {
            Information("An error occurred while attempting to create this data. Please consider re-logging or try again later.");
            return;
        }

        if (role == 1) {
            if (finddisplay->second->currentcandidate >= finddisplay->second->maxcandidate) {
                Information("Candidate already reached maximum");
                return;
            }
        }
        else if (role == 2) {
            if (finddisplay->second->currentvoter >= finddisplay->second->maxvoter) {
                Information("Voter already reached maximum");
                return;
            }
        }
        registerDialog->nameLineEdit->clear();
        registerDialog->idCardLineEdit->clear();
        registerDialog->affiliateLineEdit->clear();
        registerDialog->descLineEdit->clear();
        Block::Message msg;
        memcpy(msg.publickey, storedKey->key.publickey, 32);
        Utility::hexstring_to_uint8_t(msg.hashvotepoll, 32, registerDialog->hvp.toStdString());
        std::string qmsg = name.toStdString() + idcard.toStdString() + affiliate.toStdString() + info.toStdString();
        uint8_t header[10]{};
        Utility::int_to_uint8_t(role, header, 2);
        Utility::int_to_uint8_t(name.length(), header + 2, 2);
        Utility::int_to_uint8_t(idcard.length(), header + 4, 2);
        Utility::int_to_uint8_t(affiliate.length(), header + 6, 2);
        Utility::int_to_uint8_t(info.length(), header + 8, 2);
        long long sizemsg = qmsg.length() + 10;
        uint8_t* body = new uint8_t[sizemsg]{};
        memcpy_s(body,sizemsg, header, 10);
        memcpy_s(body + 10, sizemsg, qmsg.data(), qmsg.length());
        msg.cipher = new AES_Data;
        aes_encrypt(storedKey->key.privatekey, genesisVotepoll->publickey, body, sizemsg, msg.cipher);
        Block::calculate_msg_signature(&msg, msg.signature, &storedKey->key);
        Block::Data bd;
        Block::msg_to_blockdata(&msg, bd);
        long long thissize = bd.datsize + 4;
        auto findtheme = panelWidget->set_list_theme.find(registerDialog->hvp.toStdString());
        if(findtheme != panelWidget->set_list_theme.end()){
            findtheme->second->requestFlag = true;
            findtheme->second->setLabelColor(2);
        }
        QByteArray sendthis = QByteArray((char*)&bd.data[0], thissize);
        Information("Request Has been seen, Please wait for admin confirmation.");
        _mempool->receiveNewData(&sendthis);
        HistoryItem* theme = new HistoryItem;
        if (role == 1) {
            if (finddisplay != panelWidget->set_list_theme.end()) {
                theme->detailLabel->setText("Requested as candidate at " + finddisplay->second->pollLabel->text());
            }
        }
        else if(role == 2){
            if (finddisplay != panelWidget->set_list_theme.end()) {
                theme->detailLabel->setText("Requested as voter at " + finddisplay->second->pollLabel->text());
            }
        }
        theme->dateLabel->setText(QDateTime::currentDateTime().toString("h:mm AP\nddd, M/d/yyyy"));
        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(theme->sizeHint());
        panelWidget->historyWidget->listWidget->insertItem(0,item);
        panelWidget->historyWidget->listWidget->setItemWidget(item, theme);
        panelWidget->historyWidget->set_list_theme_req.emplace(registerDialog->hvp.toStdString(), theme);
        panelWidget->historyWidget->set_list_item_req.emplace(registerDialog->hvp.toStdString(), item);
        delete[] body;
        registerDialog->close();
        });

    connect(registerDialog->cancelButton, &QPushButton::clicked, [&]() {
        registerDialog->close();
        });

    addDialog = new AddDialog;
    addDialog->setWindowIcon(dialogIcon);
    connect(addDialog->button, &QPushButton::clicked, [&]() {
        if (storedKey == nullptr) {
            Information("There was an error while adding this data, please try re-login");
            stackedWidget->setCurrentWidget(loginWidget);
            return;
        }
        if (sodium_memcmp(storedKey->key.publickey, genesisVotepoll->publickey, 32) != 0) {
            Information("There was an error while adding this data, please try re-login");
            delete storedKey;
            storedKey = nullptr;
            stackedWidget->setCurrentWidget(loginWidget);
            return;
        }
        QString name = addDialog->candidateLineEdit->text();
        QString pk = addDialog->candidatePkLineEdit->text();
        int roleDialog = addDialog->roleComboBox->currentIndex();
        QRegularExpression hexRegex("^[0-9A-Fa-f]+$");
        if (roleDialog == 0) {
            if (name.isEmpty()) {
                Information("Candidate name is empty");
                return;
            }
            if (name.length() >= 32) {
                Information("Candidate name is too long (maximum 32 characters)");
                return;
            }
            if (pk.isEmpty()) {
                Information("Candidate publickey is empty");
                return;
            }
            if (pk.length() != 64 || !hexRegex.match(pk).hasMatch()) {
                Information("Candidate publickey should be 32 bytes of hex (64 characters)");
                return;
            }
            Block::DataCandidate dc;
            Utility::hexstring_to_uint8_t(dc.hashvotepoll, 32, addDialog->hvp.toStdString());
            Utility::ascii_to_uint8_t(name.toStdString(), dc.candidatename, 32);
            Utility::hexstring_to_uint8_t(dc.publickey, 32, pk.toStdString());
            if (crypto_core_ristretto255_is_valid_point(dc.publickey) == 0) {
                Information("Candidate publickey invalid");
                return;
            }
            uint8_t rawdc[BLOCK_DATACANDIDATE_BYTES]{};
            Block::datacandidate_to_raw(&dc, rawdc);
            QByteArray qrawdc = QByteArray((char*)&rawdc[0], BLOCK_DATACANDIDATE_BYTES);
            _helper->Method_Admin_Add(1, &qrawdc);
        }
        else if (roleDialog == 1) {
            if (pk.isEmpty()) {
                Information("Voter publickey is empty");
                return;
            }
            if (pk.length() != 64) {
                Information("Voter publickey should be 32 bytes of hex (64 characters)");
                return;
            }
            Block::DataPartisipan dp;
            Utility::hexstring_to_uint8_t(dp.hashvotepoll, 32, addDialog->hvp.toStdString());
            Utility::hexstring_to_uint8_t(dp.publickey, 32, pk.toStdString());
            if (crypto_core_ristretto255_is_valid_point(dp.publickey) == 0) {
                Information("Voter publickey invalid");
                return;
            }
            uint8_t rawdp[BLOCK_DATAPARTISIPAN_BYTES]{};
            Block::datapartisipan_to_raw(&dp, rawdp);
            QByteArray qrawdc = QByteArray((char*)&rawdp[0], BLOCK_DATAPARTISIPAN_BYTES);
            _helper->Method_Admin_Add(2, &qrawdc);
        }
        else {
            Information("error");
        }
        addDialog->candidateLineEdit->clear();
        addDialog->candidatePkLineEdit->clear();
        addDialog->close();
        });
    connect(addDialog->button2, &QPushButton::clicked, [&]() {
        addDialog->candidateLineEdit->clear();
        addDialog->candidatePkLineEdit->clear();
        addDialog->close();
        });
    detailDialog = new PollingDetailDialog;
    detailDialog->setWindowIcon(dialogIcon);
    connect(panelWidget, &PanelWidget::dialogHVP, this, &TRSVote::setupDialog);

    connect(panelWidget->listPoll, &QListWidget::itemSelectionChanged, [&]() {
        int index = panelWidget->listPoll->currentRow();
        if (index < 0) {
            return;
        }
        else if (index >= panelWidget->set_list_hvp.size()) {
            return;
        }
        QString hvp = panelWidget->set_list_hvp[index].c_str();

        if (auto findtheme = panelWidget->set_list_theme.find(hvp.toStdString()); findtheme != panelWidget->set_list_theme.end()) {
            panelWidget->button2->setDisabled(false);
            panelWidget->button3->setDisabled(false);
            panelWidget->button4->setDisabled(false);
            if (storedKey != nullptr) {
                findtheme->second->setLabelColor(storedKey->role);
            }
            long long timern = QDateTime::currentSecsSinceEpoch();
            timern += 150;

            if (timern >= findtheme->second->startdate) {
                panelWidget->button3->setDisabled(true);
            }
            else {
                if (findtheme->second->currentcandidate >= findtheme->second->maxcandidate && findtheme->second->currentvoter >= findtheme->second->maxvoter) {
                    panelWidget->button3->setDisabled(true);
                    findtheme->second->registerButton->setVisible(false);
                    findtheme->second->mailButton->setVisible(false);
                    findtheme->second->flagLabel->setText("Closed Registration");
                    findtheme->second->flagLabel->show();
                }
                if (findtheme->second->maxcandidate == 0 && findtheme->second->maxvoter == 0) {
                    findtheme->second->flagLabel->setText("Unconfirmed Transaction");
                    findtheme->second->flagLabel->show();
                    panelWidget->button2->setDisabled(true);
                    findtheme->second->mailButton->setVisible(false);
                }
            }
        }

        });

    connect(panelWidget->mailWidget->acceptButton, &QPushButton::clicked, [&]() {
        if (storedKey == nullptr) {
            Information("There was an error while adding this data, please try re-login");
            stackedWidget->setCurrentWidget(loginWidget);
            return;
        }
        if (sodium_memcmp(storedKey->key.publickey, genesisVotepoll->publickey, 32) != 0) {
            Information("There was an error while adding this data, please try re-login");
            delete storedKey;
            storedKey = nullptr;
            stackedWidget->setCurrentWidget(loginWidget);
            return;
        }
        int cur = panelWidget->mailWidget->listWidget->currentRow();
        if (cur == -1) {
            return;
        }
        std::string key = panelWidget->mailWidget->set_list_hvppk[cur];
        auto findtheme = panelWidget->mailWidget->set_list_theme.find(key);
        if (findtheme == panelWidget->mailWidget->set_list_theme.end()) {
            Information("Error while adding data, please try again later");
            return;
        }
        std::string pk = findtheme->second->pk.toStdString();
        std::string name = findtheme->second->qname.toStdString();
        std::string hvp = panelWidget->mailWidget->hvp;
        int role = findtheme->second->irole;
        if (role == 1) {
            Block::DataCandidate dc;
            Utility::hexstring_to_uint8_t(dc.publickey, 32, pk);
            Utility::hexstring_to_uint8_t(dc.hashvotepoll, 32, hvp);
            Utility::ascii_to_uint8_t(name, dc.candidatename, 32);
            uint8_t dcraw[BLOCK_DATACANDIDATE_BYTES]{};
            Block::datacandidate_to_raw(&dc, dcraw);
            QByteArray temp = QByteArray((char*)&dcraw[0], BLOCK_DATACANDIDATE_BYTES);
            _helper->Method_Admin_Add(3, &temp);
        }
        else if (role == 2) {
            Block::DataPartisipan dp;
            Utility::hexstring_to_uint8_t(dp.publickey, 32, pk);
            Utility::hexstring_to_uint8_t(dp.hashvotepoll, 32, hvp);
            uint8_t dpraw[BLOCK_DATAPARTISIPAN_BYTES]{};
            Block::datapartisipan_to_raw(&dp, dpraw);
            QByteArray temp = QByteArray((char*)&dpraw[0], BLOCK_DATAPARTISIPAN_BYTES);
            _helper->Method_Admin_Add(4, &temp);
        }
        });

        voteDialog = new VoteDialog;
        voteDialog->setWindowIcon(dialogIcon);

        connect(panelWidget->mailWidget->listWidget, &QListWidget::itemSelectionChanged, [&]() {
            int cur = panelWidget->mailWidget->listWidget->currentRow();
        if (cur < 0) {
            return;
        }
        if (cur >= panelWidget->mailWidget->set_list_hvppk.size()) {
            return;
        }
        std::string key = panelWidget->mailWidget->set_list_hvppk[cur];
        auto find = panelWidget->mailWidget->set_list_theme.find(key);
        if (find == panelWidget->mailWidget->set_list_theme.end()) {
            return;
        }
        auto finddisplay = panelWidget->set_list_theme.find(panelWidget->mailWidget->hvp);
        if (finddisplay == panelWidget->set_list_theme.end()) {
            return;
        }
        panelWidget->mailWidget->nameLineEdit->setText(find->second->qname);
        panelWidget->mailWidget->idLineEdit->setText(find->second->qid);
        panelWidget->mailWidget->afLineEdit->setText(find->second->qaf);
        panelWidget->mailWidget->pkLineEdit->setText(find->second->pk);
        panelWidget->mailWidget->textEdit->setPlainText(find->second->qinf);
        panelWidget->mailWidget->dateLabel->setText(QDateTime::fromSecsSinceEpoch(find->second->blocktime).toString("h:mm AP\nddd, MM/dd/yyyy"));
        panelWidget->mailWidget->showMail();
        panelWidget->mailWidget->mailClose(finddisplay->second->currentcandidate, finddisplay->second->maxcandidate, finddisplay->second->currentvoter, finddisplay->second->maxvoter);
        panelWidget->mailWidget->acceptMail();
        find->second->updateTime();
            });

        connect(panelWidget->listMemberWidget->searchPushButtonCandidate, &QPushButton::clicked, [&]() {
            int index = panelWidget->listMemberWidget->searchComboboxCandidate->currentIndex();
            QString searchthis = panelWidget->listMemberWidget->hvp;
            QString keyword = panelWidget->listMemberWidget->searchLineEditCandidate->text();
            if (keyword.isEmpty()) {
                Information("Please provide keyword to search");
                return;
            }
            panelWidget->listMemberWidget->clearListCandidate();
            searchthis += keyword;
            panelWidget->listMemberWidget->searchPushButtonCandidate->setDisabled(true);
            panelWidget->listMemberWidget->searchLineEditCandidate->setDisabled(true);
            panelWidget->listMemberWidget->searchComboboxCandidate->setDisabled(true);
            panelWidget->listMemberWidget->headerListWidgetCandidateFrame->setVisible(false);
            panelWidget->listMemberWidget->searchKeywordCandidateLabel->setText("has found with keyword " + keyword);
            panelWidget->listMemberWidget->candidateStackedMain->setCurrentWidget(panelWidget->listMemberWidget->pleaseWaitCandidateFrame);
            if (index == 0) {
                _helper->Method_Admin_List(2, searchthis);
            }
            else if (index == 1) {
                _helper->Method_Admin_List(3, searchthis);
            }
            });
        connect(panelWidget->listMemberWidget->searchPushButtonVoter, &QPushButton::clicked, [&]() {
            QString searchthis = panelWidget->listMemberWidget->hvp;
            QString keyword = panelWidget->listMemberWidget->searchLineEditVoter->text();
            if (keyword.isEmpty()) {
                Information("Please provide keyword to search");
                return;
            }
            panelWidget->listMemberWidget->searchPushButtonVoter->setDisabled(true);
            panelWidget->listMemberWidget->searchLineEditVoter->setDisabled(true);
            panelWidget->listMemberWidget->headerListWidgetVoterFrame->setVisible(false);
            panelWidget->listMemberWidget->searchKeywordVoterLabel->setText("has found with keyword " + keyword);
            panelWidget->listMemberWidget->voterStackedMain->setCurrentWidget(panelWidget->listMemberWidget->pleaseWaitVoterFrame);
            searchthis += keyword;
            panelWidget->listMemberWidget->clearListVoter();
            _helper->Method_Admin_List(4, searchthis);
            });
        connect(panelWidget->listMemberWidget->refreshCandidateButton, &QPushButton::clicked, [&]() {
            panelWidget->listMemberWidget->refreshCandidateButton->setDisabled(true);
        panelWidget->listMemberWidget->searchPushButtonCandidate->setDisabled(true);
        panelWidget->listMemberWidget->searchLineEditCandidate->setDisabled(true);
        panelWidget->listMemberWidget->searchComboboxCandidate->setDisabled(true);
        panelWidget->listMemberWidget->headerListWidgetCandidateFrame->setVisible(false);
        panelWidget->listMemberWidget->candidateStackedMain->setCurrentWidget(panelWidget->listMemberWidget->pleaseWaitCandidateFrame);
            _helper->Method_Admin_List(5, panelWidget->listMemberWidget->hvp);
            });

        connect(panelWidget->listMemberWidget->refreshVoterButton, &QPushButton::clicked, [&]() {
            panelWidget->listMemberWidget->refreshVoterButton->setDisabled(true);
        panelWidget->listMemberWidget->searchPushButtonVoter->setDisabled(true);
        panelWidget->listMemberWidget->searchLineEditVoter->setDisabled(true);
        panelWidget->listMemberWidget->headerListWidgetVoterFrame->setVisible(false);
        panelWidget->listMemberWidget->voterStackedMain->setCurrentWidget(panelWidget->listMemberWidget->pleaseWaitVoterFrame);
        _helper->Method_Admin_List(6, panelWidget->listMemberWidget->hvp);
            });
}

void TRSVote::handleRegister(QString hvp) {
    registerDialog->roleComboBox->setCurrentIndex(0);
    registerDialog->hvp = hvp;
    registerDialog->exec();
}

bool TRSVote::checkThatAccountValid(QString username, QString password) {
    if (username.length() < 8) {
        Information(QString("Username must be at least 8 characters long."));
        return false;
    }
    if (password.length() < 8) {
        Information(QString("Password must be at least 8 characters long."));
        return false;
    }
    QRegularExpression numberRegex("\\d");
    QRegularExpressionMatch numberMatch = numberRegex.match(password);

    QRegularExpression uppercaseRegex("[A-Z]");
    QRegularExpressionMatch uppercaseMatch = uppercaseRegex.match(password);

    bool hasNumber = numberMatch.hasMatch();
    bool hasUppercase = uppercaseMatch.hasMatch();
    if (!hasNumber || !hasUppercase) {
        Information(QString("Password must contain at least one number and one uppercase letter."));
        return false;
    }
    return true;
}

bool TRSVote::checkThatKeypairValid(QString privatekey) {
    if (privatekey.length() != 64) {
        Information(QString("Private key must be 64 characters long."));
        return false;
    }

    QRegularExpression hexRegex("^[0-9a-fA-F]+$");
    if (!hexRegex.match(privatekey).hasMatch()) {
        Information(QString("Private key must be in hexadecimal format."));
        return false;
    }

    uint8_t scalar[32]{};
    Utility::hexstring_to_uint8_t(scalar, 32, privatekey.toStdString());
    if (!is_valid_scalar(scalar)) {
        Information(QString("Private key is invalid scalar."));
        return false;
    }
    return true;
}

void TRSVote::Information(QString msg) {
    QDialog dialog;
    QIcon iconui(":/TRSVote/styles/image/dialogico.png");
    dialog.setWindowIcon(iconui);
    dialog.setWindowTitle("TRS Vote");
    dialog.setStyleSheet("background-color: #F5F8FA; border: none; border-radius: 5px;");

    QVBoxLayout layout = QVBoxLayout(&dialog);

    QLabel msgLabel = QLabel(msg, &dialog);
    QFont font("Segoe UI", 12);  // Specify the desired font family and size
    msgLabel.setFont(font);
    msgLabel.setWordWrap(true);
    msgLabel.setAlignment(Qt::AlignCenter);
    // Calculate the required width based on the text's size
    int textWidth = msgLabel.fontMetrics().boundingRect(msgLabel.text()).width();

    // Set the width with some additional padding
    int msgBoxWidth = textWidth + 100;
    layout.addWidget(&msgLabel);

    QPushButton okButton = QPushButton("OK", &dialog);
    okButton.setStyleSheet(
        "QPushButton {"
        "background-color: #1DA1F2;"
        "border-radius: 20px;"
        "color: #FFFFFF;"
        "padding: 5px 20px;"
        "}"
        "QPushButton:hover {"
        "background-color: #0F8BDD;"
        "}"
        "QPushButton:pressed {"
        "background-color: #0A5F95;"
        "}");
    layout.addWidget(&okButton, 0, Qt::AlignRight);
    connect(&okButton, &QPushButton::clicked, [&]() {
        dialog.close();
    dialog.deleteLater();
        });
    dialog.setFixedSize(msgBoxWidth, dialog.sizeHint().height());
    dialog.exec();

}

void TRSVote::SetupMempool() {
    QIcon icon(":/TRSVote/styles/image/dialogico.png");
    nsdialog = new NetworkDialog;
    nsdialog->setWindowIcon(icon);
    nsdialog->GettingPublicConn();
    connect(ui.actionNetwork, &QAction::triggered, [&]() {
        nsdialog->DefaultPort(); nsdialog->exec();
        });
    _helper = new Helper;
    _helperthread = new QThread;
    _helper->moveToThread(_helperthread);
    connect(_helperthread, &QThread::started, _helper, &Helper::main, Qt::QueuedConnection);
    connect(_helper, &Helper::finished, _helperthread, &QThread::quit, Qt::DirectConnection);
    _helperthread->start();
    connect(_helper, &Helper::Information, this, &TRSVote::Information, Qt::QueuedConnection);
    connect(_helper, &Helper::loginMethod, this, &TRSVote::HelperLogin, Qt::QueuedConnection);
    connect(_helper, &Helper::setFlagMethod, this, &TRSVote::setFlagHelper, Qt::QueuedConnection);
    connect(_helper, &Helper::setHistory, this, &TRSVote::setHistoryHelper, Qt::QueuedConnection);
    connect(_helper, &Helper::HelperToMemPoolNewData, this, &TRSVote::HelperToMemPoolNewData, Qt::QueuedConnection);
    connect(_helper, &Helper::AddMoreSize, this, &TRSVote::HelperAddMoreSize, Qt::QueuedConnection);
    connect(_helper, &Helper::setMail, this, &TRSVote::setMailHelper, Qt::QueuedConnection);
    connect(_helper, &Helper::setList, this, &TRSVote::setListHelper, Qt::QueuedConnection);
    connect(_helper, &Helper::setResult, this, &TRSVote::setResultHelper, Qt::QueuedConnection);
    connect(_helper, &Helper::setVote, this, &TRSVote::setVoteHelper, Qt::QueuedConnection);
    connect(_helper, &Helper::ConnectionInfo, this, &TRSVote::ConnectionInfo, Qt::QueuedConnection);
    _mempool = new MemPool;
    _mempoolthread = new QThread;
    _mempool->moveToThread(_mempoolthread);
    connect(_mempoolthread, SIGNAL(started()), _mempool, SLOT(main()), Qt::QueuedConnection);
    connect(_mempool, SIGNAL(finished()), _mempoolthread, SLOT(quit()), Qt::DirectConnection);
    _mempoolthread->start();
    connect(_mempool, &MemPool::Update_BlockHeight, this, &TRSVote::Update_BlockHeight, Qt::QueuedConnection);
    connect(_mempool, &MemPool::callGenesisForm, this, &TRSVote::callGenesisForm, Qt::QueuedConnection);
    connect(_mempool, &MemPool::setupVotepollGenesis, this, &TRSVote::setupVotepollGenesis);
    //connect(_mempool, &MemPool::loginData, this, &TRSVote::handleLogin);
    //connect(_mempool, &MemPool::reset_display, this, &TRSVote::resetPolling);
    connect(_mempool, &MemPool::send_Info_to_LoadingBar, this, &TRSVote::send_Info_to_LoadingBar, Qt::QueuedConnection);
    //connect(_mempool, &MemPool::send_display, this, &TRSVote::updatePolling);
    connect(_mempool, &MemPool::setupGenesisVotepollHelper, this, &TRSVote::setupVotepollGenesis, Qt::QueuedConnection);
    connect(_mempool, &MemPool::parseBlockHelper, this, &TRSVote::parseBlockHelper, Qt::QueuedConnection);

    connect(_mempool, &MemPool::parseBlockFromBegHelper, this, &TRSVote::parseBlockFromBegHelper, Qt::QueuedConnection);

    connect(_mempool, &MemPool::removePeer, this, &TRSVote::removePeer, Qt::QueuedConnection);
    QString settingpath = "settings.ini";
    QFile r(settingpath);
    if (r.exists()) {
        r.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text);
        QTextStream in(&r);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.contains("genesissync")) {
                if (line.contains("true")) {
                    nsdialog->protectConnCheckBox->setChecked(true);
                    _mempool->changeFlagSync(true);
                }
                else if (line.contains("false")) {
                    nsdialog->protectConnCheckBox->setChecked(false);
                    _mempool->changeFlagSync(false);
                }
            }
            else if (line.contains("validating")) {
                if (line.contains("true")) {
                    nsdialog->supportValidatorCheckBox->setChecked(true);
                    _mempool->Method_Validating(2);
                }
                else if (line.contains("false")) {
                    nsdialog->supportValidatorCheckBox->setChecked(false);
                    _mempool->Method_Validating(1);
                }
            }
        }
        r.close();
    }
    else {
        r.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text);
        QTextStream in(&r);
        QString line = "genesissync = true\nvalidating = false";;
        in << line;
        r.close();
        nsdialog->protectConnCheckBox->setChecked(true);
        _mempool->changeFlagSync(true);
        nsdialog->supportValidatorCheckBox->setChecked(false);
    }
    connect(nsdialog, &NetworkDialog::Information, this, &TRSVote::Information);
    connect(nsdialog->saveButtonMoreSettings, &QPushButton::clicked, [=]() {
        bool genesissync = nsdialog->protectConnCheckBox->isChecked();
        bool validating = nsdialog->supportValidatorCheckBox->isChecked();
        QString settingpaths = "settings.ini";
        QFile rs(settingpaths);
        if (rs.exists()) {
            rs.open(QIODeviceBase::ReadWrite | QIODeviceBase::Text);
            QTextStream in(&rs);
            QString fileContent;
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.contains("genesissync")) {
                    if (genesissync) {
                        line.replace("false", "true");
                        _mempool->changeFlagSync(true);
                    }
                    else {
                        line.replace("true", "false");
                        _mempool->changeFlagSync(false);
                    }
                }
                else if (line.contains("validating")) {
                    if (validating) {
                        line.replace("false", "true");
                        _mempool->Method_Validating(2);
                    }
                    else {
                        line.replace("true", "false");
                        _mempool->Method_Validating(1);
                    }
                }
                fileContent += line + "\n";
            }
            rs.resize(0);
            in.seek(0);
            in << fileContent;
            rs.close();
            nsdialog->close();
        }
        Information("Save changes success.");
        });

    connect(nsdialog, &NetworkDialog::tryConn, this, &TRSVote::tryConn);

    connect(nsdialog, &NetworkDialog::changePeer, [&](int peer) {
        if (peer == 0) {
            peerlabel->setText("Peer : No Connection");
        }
        else {
            peerlabel->setText("Peer : " + QString::number(peer));
        }
        });

    connect(nsdialog, &NetworkDialog::DoHandshake, [&](long long key) {
        _mempool->DoHandshake(key);
        });

    connect(nsdialog, &NetworkDialog::DoBreakup, [&](long long key) {
        _mempool->breakUp(key);
        });
    connect(nsdialog, &NetworkDialog::DoReceiveHandshake, [&](long long key, QByteArray dat) {
        _mempool->receiveHandshake(key, dat);
        });
    connect(nsdialog, &NetworkDialog::DoRequestBlockFork, [&](long long key, QByteArray dat) {
        _mempool->requestBlockFork(key, dat);
        });
    connect(nsdialog, &NetworkDialog::DoReceiveBlockFork, [&](long long key, QByteArray dat) {
        _mempool->receiveBlockFork(key, dat);
        });
    connect(nsdialog, &NetworkDialog::DoReceiveBlockHeader, [&](long long key, QByteArray dat) {
    _mempool->receiveBlockHeader(key, dat);
        });
    connect(nsdialog, &NetworkDialog::DoReceiveSyncStatus, [&](long long key, QByteArray dat) {
        _mempool->receiveSyncStatus(key, dat);
        });
    connect(nsdialog, &NetworkDialog::DoRequestBlockByHash, [&](long long key, QByteArray dat) {
        _mempool->requestBlockByHash(key, dat);
        });
    connect(nsdialog, &NetworkDialog::DoReceiveNewBlock, [&](QByteArray dat) {
        _mempool->receiveNewBlock(&dat);
        });
    connect(nsdialog, &NetworkDialog::DoReceiveNewData, [&](QByteArray dat) {
        _mempool->receiveNewData(&dat);
        });
    nsdialog->setupServer();

    connect(_mempool, &MemPool::sendMethod, this, &TRSVote::sendMethod);

}

void TRSVote::tryConn(QString ipAddress, long long port, int con) {
    _helper->tryConn(ipAddress, port, con);
}

void TRSVote::ConnectionInfo(QString ip, long long port, bool con, int withdialog) {
    nsdialog->ConnectionInfo(ip, port, con, withdialog);
}

void TRSVote::sendMethod(long long key, QByteArray dat) {
    nsdialog->sendMethod(key, dat);
}

void TRSVote::callGenesisForm() {
    loadingBar->hide();
    stackedWidget->setCurrentWidget(registerWidget);
}

void TRSVote::setupVotepollGenesis(Block::VotePoll vp) {
    _helper->setupGenesisVotepoll(&vp);
    if (genesisVotepoll != nullptr) {
        delete genesisVotepoll;
    }
    registerWidget->endWait();
    genesisVotepoll = new Block::VotePoll;
    memcpy(genesisVotepoll, &vp, sizeof(Block::VotePoll));
    if (storedKey != nullptr) {
        panelWidget->setupPanel(storedKey->role);
        if (sodium_memcmp(genesisVotepoll->publickey, storedKey->key.publickey, 32) == 0) {
            storedKey->role = 1;
            panelWidget->usernameDataLabel->setText(storedKey->username.c_str());
            panelWidget->setupPanel(1);
            stackedWidget->setCurrentWidget(panelWidget);
        }
        else {
            delete storedKey;
            storedKey = nullptr;
            Information("It appears that the system has encountered a fork. Please proceed to re-login to ensure a seamless experience.");
            panelWidget->logoutPanel();
            stackedWidget->setCurrentWidget(loginWidget);
        }
    }
    else {
        stackedWidget->setCurrentWidget(loginWidget);
    }
}

void TRSVote::handleLogin(int m, std::string hvp, VoteTree::history h) {
    if (m == 2) {
        if (h.ciphermsg.empty() || h.datemsg.empty()) {
            return;
        } 
        std::string key = hvp + h.ciphermsg.begin()->first;
        if (auto find = panelWidget->mailWidget->set_list_theme.find(key); find != panelWidget->mailWidget->set_list_theme.end()) {
            find->second->cipher = h.ciphermsg.begin()->second;
            find->second->blocktime = h.datemsg.begin()->second;
        }
        else {
            MessageListWidget* widget = new MessageListWidget;
            QListWidgetItem* item = new QListWidgetItem;
            auto finddesc = panelWidget->set_list_theme.find(hvp);
            if (finddesc != panelWidget->set_list_theme.end()) {
                widget->desc = finddesc->second->pollLabel->text();
                widget->hvpstartdate = finddesc->second->startdate;
            }
            widget->cipher = h.ciphermsg.begin()->second;
            widget->blocktime = h.datemsg.begin()->second;
            widget->pk = h.ciphermsg.begin()->first.c_str();
            item->setSizeHint(widget->sizeHint());
            panelWidget->mailWidget->listWidget->insertItem(0, item);
            panelWidget->mailWidget->listWidget->setItemWidget(item, widget);
            panelWidget->mailWidget->set_list_theme.emplace(key, widget);
            panelWidget->mailWidget->set_list_item.emplace(key, item);
            panelWidget->mailWidget->set_list_hvppk.push_front(key);
        }
        if (storedKey != nullptr) {
            if (storedKey->role == 1) {
                auto fint = panelWidget->mailWidget->set_list_theme.find(key);
                if (fint == panelWidget->mailWidget->set_list_theme.end()) {
                    return;
                }
                fint->second->openCipher(storedKey->key.privatekey);
            }
            else if (storedKey->role == 2) {
                uint8_t upk[32]{};
                Utility::hexstring_to_uint8_t(upk, 32, h.ciphermsg.begin()->first);
                if (sodium_memcmp(storedKey->key.publickey, upk, 32) != 0) {
                    return;
                }
                if (auto fintheme = panelWidget->historyWidget->set_list_theme_req.find(hvp); fintheme != panelWidget->historyWidget->set_list_theme_req.end()) {
                    fintheme->second->dateLabel->setText(QDateTime::fromSecsSinceEpoch(h.datemsg.begin()->second).toString("h:mm AP\nddd, M/d/yyyy"));
                    return;
                }
                auto fint = panelWidget->mailWidget->set_list_theme.find(key);
                if (fint == panelWidget->mailWidget->set_list_theme.end()) {
                    return;
                }
                fint->second->openCipherAsUser(&storedKey->key, genesisVotepoll->publickey);
                HistoryItem* theme = new HistoryItem;
                if (fint->second->irole == 1) {
                    theme->detailLabel->setText("Requested as candidate at " + fint->second->desc);
                }
                else {
                    theme->detailLabel->setText("Requested as voter at " + fint->second->desc);
                }
                QListWidgetItem* item = new QListWidgetItem;
                item->setSizeHint(theme->sizeHint());
                panelWidget->historyWidget->listWidget->insertItem(0,item);
                panelWidget->historyWidget->listWidget->setItemWidget(item, theme);
                panelWidget->historyWidget->set_list_item_req.emplace(hvp, item);
                panelWidget->historyWidget->set_list_theme_req.emplace(hvp, theme);
                if (auto finclick = panelWidget->set_list_theme.find(hvp); finclick != panelWidget->set_list_theme.end()) {
                    finclick->second->requestFlag = true;
                }
            }
        }
    }
    else if (m == 5) {
        if (h.updatedatemsgaccepted.empty()) {
            return;
        }
        std::string key = hvp + h.updatedatemsgaccepted.begin()->first;
        if (auto find = panelWidget->mailWidget->set_list_theme.find(key); find != panelWidget->mailWidget->set_list_theme.end()) {
            find->second->acceptedtime = h.updatedatemsgaccepted.begin()->second;
        }
        if (storedKey != nullptr) {
            if (storedKey->role == 1) {
                auto fint = panelWidget->mailWidget->set_list_theme.find(key);
                if (fint == panelWidget->mailWidget->set_list_theme.end()) {
                    return;
                }
                fint->second->openCipher(storedKey->key.privatekey);
                fint->second->updateTime();
            }
            /*else if (storedKey->role == 2) {
                uint8_t upk[32]{};
                Utility::hexstring_to_uint8_t(upk, 32, h.ciphermsg.begin()->first);
                if (sodium_memcmp(storedKey->key.publickey, upk, 32) != 0) {
                    return;
                }
                if (panelWidget->historyWidget->set_list_theme_reg.find(hvp) != panelWidget->historyWidget->set_list_theme_reg.end()) {
                    return;
                }
                auto fint = panelWidget->mailWidget->set_list_theme.find(key);
                if (fint == panelWidget->mailWidget->set_list_theme.end()) {
                    return;
                }
                fint->second->openCipherAsUser(&storedKey->key, genesisVotepoll->publickey);
                HistoryItem* theme = new HistoryItem;
                if (fint->second->irole == 1) {
                    theme->detailLabel->setText("Registered as candidate at " + fint->second->desc);
                }
                else {
                    theme->detailLabel->setText("Registered as voter at " + fint->second->desc);
                }
                QListWidgetItem* item = new QListWidgetItem;
                item->setSizeHint(theme->sizeHint());
                panelWidget->historyWidget->listWidget->addItem(item);
                panelWidget->historyWidget->listWidget->setItemWidget(item, theme);
                panelWidget->historyWidget->set_list_item_reg.emplace(hvp, item);
                panelWidget->historyWidget->set_list_theme_reg.emplace(hvp, theme);
            }*/
        }
    }
    else if (m == 7) {
        for (auto it = panelWidget->mailWidget->set_list_theme.begin(); it != panelWidget->mailWidget->set_list_theme.end(); it++) {
            MessageListWidget* temp = nullptr;
            std::swap(temp, it->second);
            delete temp;
        }
        panelWidget->mailWidget->set_list_theme.clear();
        for (auto it = panelWidget->mailWidget->set_list_item.begin(); it != panelWidget->mailWidget->set_list_item.end(); it++) {
            QListWidgetItem* temp = nullptr;
            std::swap(temp, it->second);
            delete temp;
        }
        panelWidget->mailWidget->set_list_item.clear();
        panelWidget->mailWidget->set_list_hvppk.clear();
        panelWidget->mailWidget->listWidget->clear();
        if (storedKey != nullptr) {
            Information("It appears that the system has encountered a fork. Please proceed to re-login to ensure a seamless experience.");
            delete storedKey;
            storedKey = nullptr;
            stackedWidget->setCurrentWidget(loginWidget);
        }
    }
}

void TRSVote::updatePolling(int m, QString hvp, VoteTree::display s) {
    if (m == 1) {
        std::string key = hvp.toStdString();
        if (auto find = set_display.find(key); find != set_display.end()) {
            find->second->vp = s.vp;
        }
        else {
            set_display.emplace(key, new VoteTree::display(s));
        }
        if (auto find = panelWidget->set_list_theme.find(key); find != panelWidget->set_list_theme.end()) {
            find->second->display = s;
            find->second->startdate = s.vp.startdate;
            find->second->enddate = s.vp.enddate;
        }
        else {
            ClickableLayoutWidget* newlayout = new ClickableLayoutWidget;
            newlayout->display = s;
            QString desc = s.vp.desc;
            newlayout->hashvotepoll = hvp;
            newlayout->pollLabel->setText(desc);
            newlayout->startdate = s.vp.startdate;
            newlayout->enddate = s.vp.enddate;
            if (storedKey != nullptr) {
                newlayout->setLabelColor(storedKey->role);
            }
            else {
                newlayout->setLabelColor(2);
            }
            QListWidgetItem* newlist = new QListWidgetItem;
            newlist->setSizeHint(newlayout->sizeHint());
            panelWidget->listPoll->insertItem(0, newlist);
            panelWidget->listPoll->setItemWidget(newlist, newlayout);
            panelWidget->set_list_theme.emplace(key, newlayout);
            panelWidget->set_list_item.emplace(key, newlist);
            panelWidget->set_list_hvp.push_front(key);
            connect(newlayout, &ClickableLayoutWidget::registerHVP, this, &TRSVote::handleRegister);
            connect(newlayout, &ClickableLayoutWidget::voteHVP, this, &TRSVote::handleVote);
            connect(newlayout, &ClickableLayoutWidget::resultHVP, this, &TRSVote::handleResult);
        }
        panelWidget->updateDatePolling();
    }
    else if (m == 2) {
        if (s.dc.isEmpty()) {
            return;
        }
        std::string key = hvp.toStdString();
        if (auto find = set_display.find(key); find != set_display.end()) {
            if (auto findpk = find->second->dc_index.find(s.dc.front().pk.toStdString()); findpk != find->second->dc_index.end()) {
                int index = findpk->second;
                find->second->dc[index] = s.dc.front();
            }
            else {
                find->second->dc.push_back(s.dc.front());
                int index = find->second->dc.size() - 1;
                find->second->dc_index.emplace(s.dc.front().pk.toStdString(), index);
                uint8_t pktemp[32]{};
                Utility::hexstring_to_uint8_t(pktemp,32, s.dc.front().pk.toStdString());
                if (storedKey != nullptr) {
                    if (storedKey->role == 2 && sodium_memcmp(storedKey->key.publickey,pktemp, 32) == 0) {
                        if (auto fintheme = panelWidget->historyWidget->set_list_theme_reg.find(hvp.toStdString()); fintheme != panelWidget->historyWidget->set_list_theme_reg.end()) {
                            fintheme->second->dateLabel->setText(QDateTime::fromSecsSinceEpoch(s.dc.begin()->blocktime).toString("h:mm AP\nddd, M/d/yyyy"));
                            return;
                        }
                        HistoryItem* theme = new HistoryItem;
                        theme->detailLabel->setText("Registered as candidate at " + find->second->vp.desc);
                        theme->dateLabel->setText(QDateTime::fromSecsSinceEpoch(find->second->dc[index].blocktime).toString("h:mm AP\nddd, M/d/yyyy"));
                        QListWidgetItem* item = new QListWidgetItem;
                        item->setSizeHint(theme->sizeHint());
                        panelWidget->historyWidget->listWidget->insertItem(0, item);
                        panelWidget->historyWidget->listWidget->setItemWidget(item, theme);
                        panelWidget->historyWidget->set_list_theme_reg.emplace(find->first, theme);
                        panelWidget->historyWidget->set_list_item_reg.emplace(find->first, item);
                        if (auto findclick = panelWidget->set_list_theme.find(hvp.toStdString()); findclick != panelWidget->set_list_theme.end()) {
                            findclick->second->registeredFlag = true;
                            findclick->second->asCandidateFlag = true;
                            findclick->second->setLabelColor(2);
                        }
                    }
                }
            }

        }
    }
    else if (m == 3) {
        if (s.dp.isEmpty()) {
            return;
        }
        std::string key = hvp.toStdString();
        if (auto find = set_display.find(key); find != set_display.end()) {
            if (auto findpk = find->second->dp_index.find(s.dp.front().pk.toStdString()); findpk != find->second->dp_index.end()) {
                int index = findpk->second;
                find->second->dp[index] = s.dp.front();
            }
            else {
                find->second->dp.push_back(s.dp.front());
                int index = find->second->dp.size() - 1;
                find->second->dp_index.emplace(s.dp.front().pk.toStdString(), index);
                uint8_t pktemp[32]{};
                Utility::hexstring_to_uint8_t(pktemp, 32, s.dp.front().pk.toStdString());
                if (storedKey != nullptr) {
                    if (storedKey->role == 2 && sodium_memcmp(storedKey->key.publickey, pktemp, 32) == 0) {
                        if (auto fintheme = panelWidget->historyWidget->set_list_theme_reg.find(hvp.toStdString()); fintheme != panelWidget->historyWidget->set_list_theme_reg.end()) {
                            fintheme->second->dateLabel->setText(QDateTime::fromSecsSinceEpoch(s.dp.begin()->blocktime).toString("h:mm AP\nddd, M/d/yyyy"));
                            return;
                        }
                        HistoryItem* theme = new HistoryItem;
                        theme->detailLabel->setText("Registered as voter at " + find->second->vp.desc);
                        theme->dateLabel->setText(QDateTime::fromSecsSinceEpoch(find->second->dp[index].blocktime).toString("h:mm AP\nddd, M/d/yyyy"));
                        QListWidgetItem* item = new QListWidgetItem;
                        item->setSizeHint(theme->sizeHint());
                        panelWidget->historyWidget->listWidget->insertItem(0, item);
                        panelWidget->historyWidget->listWidget->setItemWidget(item, theme);
                        panelWidget->historyWidget->set_list_theme_reg.emplace(find->first, theme);
                        panelWidget->historyWidget->set_list_item_reg.emplace(find->first, item);
                        if (auto findclick = panelWidget->set_list_theme.find(hvp.toStdString()); findclick != panelWidget->set_list_theme.end()) {
                            findclick->second->registeredFlag = true;
                            findclick->second->nonVFlag = false;
                            findclick->second->setLabelColor(2);
                        }
                    }
                }
            }
        }
    }
    else if (m == 4) {
        if (s.vote.isEmpty() || s.vote_index.empty()) {
            return;
        }
        std::string key = hvp.toStdString();
        if (auto findd = set_display.find(key); findd != set_display.end()) {
            int voteinde = s.vote.begin()->vote_a1.begin()->second;
            findd->second->dc[voteinde].totalvote++;
            if (auto findv = findd->second->vote_index.find(s.vote_index.begin()->first); findv != findd->second->vote_index.end()) {
                VoteTree::VoteString* temp = &findd->second->vote[findv->second];
                if (auto fina1 = temp->vote_a1.find(s.vote.begin()->vote_a1.begin()->first); fina1 == temp->vote_a1.end()) {
                    temp->vote_a1.emplace(s.vote.begin()->vote_a1.begin()->first, s.vote.begin()->vote_a1.begin()->second);
                    temp->block_time.emplace(s.vote.begin()->block_time.begin()->first, s.vote.begin()->block_time.begin()->second);
                }
                else {
                    fina1->second = s.vote.begin()->vote_a1.begin()->second;
                    temp->block_time.find(s.vote.begin()->block_time.begin()->first)->second = s.vote.begin()->block_time.begin()->second;
                }
            }
            else {
                VoteTree::VoteString temp;
                temp.vote_a1.emplace(s.vote.begin()->vote_a1.begin()->first, s.vote.begin()->vote_a1.begin()->second);
                temp.block_time.emplace(s.vote.begin()->block_time.begin()->first, s.vote.begin()->block_time.begin()->second);
                findd->second->vote.push_back(temp);
                int index = findd->second->vote.size() - 1;
                findd->second->vote_index.emplace(s.vote_index.begin()->first, index);
            }
        }
    }
}

void TRSVote::resetPolling() {
    for (auto it = panelWidget->set_list_theme.begin(); it != panelWidget->set_list_theme.end(); it++) {
        ClickableLayoutWidget* temp = nullptr;
        std::swap(it->second, temp);
        delete temp;
    }
    for (auto it = panelWidget->set_list_item.begin(); it != panelWidget->set_list_item.end(); it++) {
        QListWidgetItem* temp = nullptr;
        std::swap(temp, it->second);
        delete temp;
    }
    panelWidget->set_list_theme.clear();
    panelWidget->set_list_item.clear();
    panelWidget->set_list_hvp.clear();
    panelWidget->listPoll->clear();
    for (auto it = set_display.begin(); it != set_display.end(); it++) {
        VoteTree::display* temp = nullptr;
        std::swap(it->second, temp);
        delete temp;
    }
    set_display.clear();
    if (storedKey != nullptr) {
        Information("It appears that the system has encountered a fork. Please proceed to re-login to ensure a seamless experience.");
        delete storedKey;
        storedKey = nullptr;
        stackedWidget->setCurrentWidget(loginWidget);
    }
}

void TRSVote::send_Info_to_LoadingBar(int method, int count) {
    if (method == 1) {
        loadingBar->setValue(0);
        loadingBar->show();
        statusloadingBar->setText("Validating Block");
        loadingBar->setMaximum(count);
    }
    else if (method == 2) {
        loadingBar->setValue(count);
    }
    else if (method == 3) {
        loadingBar->reset();
        loadingBar->hide();
        statusloadingBar->setText("");
    }
    else if (method == 4) {
        statusloadingBar->setText("Load BlockData");
    }
    else if (method == 6) {
        statusloadingBar->setText("Failed to validate Block.");
        loadingBar->reset();
        loadingBar->hide();
    }
    else if (method == 7) {
        loadingBar->setValue(0);
        loadingBar->show();
        statusloadingBar->setText("Getting Block");
        loadingBar->setMaximum(count);
    }
}

void TRSVote::handleVote(QString hvp) {
    voteDialog->clearlistWidget();
    voteDialog->hvp = hvp;
    if (auto findit = set_display.find(hvp.toStdString()); findit != set_display.end()) {
        voteDialog->moreLabel->setText("Choose a candidate that you will support in " + findit->second->vp.desc);
        for (int a = 0; a < findit->second->dc.size(); a++) {
            voteItem* theme = new voteItem;
            theme->hvp = hvp;
            theme->pk = findit->second->dc[a].pk;
            theme->nameLabel->setText(findit->second->dc[a].name);
            theme->pkLabel->setText("Publickey : " + findit->second->dc[a].pk);
            QListWidgetItem* item = new QListWidgetItem;
            item->setSizeHint(theme->sizeHint());
            voteDialog->listWidget->addItem(item);
            voteDialog->listWidget->setItemWidget(item, theme);
            voteDialog->listTheme.push_back(theme);
            voteDialog->listItem.push_back(item);
            connect(theme, &voteItem::doVote, this, &TRSVote::doingVote);
        }
    }
    voteDialog->exec();
}

void TRSVote::doingVote(QString hvp, QString pk) {
    auto findd = set_display.find(hvp.toStdString());
    if (findd == set_display.end()) {
        Information("Can't vote right now, please try again later");
        return;
    }
    auto finddc = findd->second->dc_index.find(pk.toStdString());
    if (finddc == findd->second->dc_index.end()) {
        Information("Can't vote right now, please try again later");
        return;
    }

    if (finddc->second < 0) {
        Information("Can't vote right now, please try again later");
        return;
    }

    if (finddc->second >= findd->second->dc.size()) {
        Information("Can't vote right now, please try again later");
        return;
    }
    QDialog dialog;
    QMessageBox warningBox(QMessageBox::Warning, "TRS Vote", "Are you sure want to choose candidate A in Vote Poll B?", QMessageBox::Yes | QMessageBox::No, &dialog);
    warningBox.setStyleSheet("background-color: #F5F8FA; border: none; border-radius: 5px;");
    warningBox.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    // Retrieve the buttons
    QAbstractButton* yesButton = warningBox.button(QMessageBox::Yes);
    QAbstractButton* noButton = warningBox.button(QMessageBox::No);

    // Modify the buttons
    yesButton->setText("Agree");
    yesButton->setStyleSheet("QPushButton {"
        "background-color: #1DA1F2;"
        "border-radius: 20px;"
        "color: #FFFFFF;"
        "padding: 5px 20px;"
        "}"
        "QPushButton:hover {"
        "background-color: #0F8BDD;"
        "}"
        "QPushButton:pressed {"
        "background-color: #0A5F95;"
        "}");

    noButton->setText("Disagree");
    noButton->setStyleSheet("QPushButton {"
        "background-color: #F44336;"
        "border-radius: 20px;"
        "color: #FFFFFF;"
        "padding: 5px 20px;"
        "}"
        "QPushButton:hover {"
        "background-color: #D32F2F;"
        "}"
        "QPushButton:pressed {"
        "background-color: #B71C1C;"
        "}");

    int response = warningBox.exec();

    if (response == QMessageBox::Yes) {
        if (storedKey == nullptr) {
            Information("There was an error when try to voting, please try re-login");
            stackedWidget->setCurrentWidget(loginWidget);
            return;
        }
        int dcindex = -1;
        if (auto finddcindex = findd->second->dc_index.find(pk.toStdString()); finddcindex != findd->second->dc_index.end()) {
            dcindex = finddcindex->second;
        }
        else {
            Information("Error when voting, please try-again later.");
            return;
        }
        std::string mypks = Utility::uint8_t_to_hexstring(storedKey->key.publickey, 32);
        TRS::Tag mytag;
        if (!findd->second->get_tag(mypks, mytag)) {
            Information("Error when voting, please try-again later.");
            return;
        }
        std::string hashhextag = VoteTree::get_hash_hex_tag(&mytag);

        // check that already vote or not;
        auto findtag = findd->second->vote_index.find(hashhextag);
        VoteTree::VoteString* vstring = nullptr;
        if (findtag != findd->second->vote_index.end()) {
            vstring = &findd->second->vote[findtag->second];
            for (int a = 0; a < findd->second->dc.size(); a++) {
                Block::DataVote votetemp;
                votetemp.tag = mytag;
                Utility::hexstring_to_uint8_t(votetemp.hashdatacandidate, 32, findd->second->dc[a].hash.toStdString());
                Block::calculate_datavote_signature(&votetemp, votetemp.signature, &storedKey->key);
                std::string a1temp = Utility::uint8_t_to_hexstring(votetemp.signature.A_1, 32);
                if (vstring->vote_a1.find(a1temp) == vstring->vote_a1.end()) {
                    continue;
                }
                else {
                    Information("Can't vote again, due already vote before");
                    return;
                }
            }
        }

        Block::DataVote myvote;
        myvote.tag = mytag;
        int indexmsg = findd->second->dc_index.find(pk.toStdString())->second;
        Utility::hexstring_to_uint8_t(myvote.hashdatacandidate, 32, findd->second->dc[indexmsg].hash.toStdString());
        Block::calculate_datavote_signature(&myvote, myvote.signature, &storedKey->key);
        Utility::int_to_uint8_t(mytag.publickey.size(), myvote.ringsize, 2);
        std::string a1string = Utility::uint8_t_to_hexstring(myvote.signature.A_1, 32);

        Block::Data bd;
        myvote.signature.ring_size = myvote.tag.publickey.size();
        Block::datavote_to_blockdata(&myvote, bd);
        int sizebd = bd.datsize + 4;
        QByteArray sendthis = QByteArray((char*)&bd.data[0], sizebd);
        _mempool->receiveNewData(&sendthis);
        if (auto findclick = panelWidget->set_list_theme.find(hvp.toStdString()); findclick != panelWidget->set_list_theme.end()) {
            findclick->second->voteFlag = true;
            findclick->second->setLabelColor(2);
        }
        if (vstring == nullptr) {
            VoteTree::VoteString newvs;
            newvs.vote_a1.emplace(a1string, dcindex);
            newvs.block_time.emplace(a1string, 0);
            findd->second->vote.push_back(newvs);
            int indexnewvs = findd->second->vote.size() - 1;
            findd->second->vote_index.emplace(hashhextag, indexnewvs);
        }
        else {
            vstring->vote_a1.emplace(a1string, dcindex);
            vstring->block_time.emplace(a1string, 0);
        }
        HistoryItem* newhistory = new HistoryItem;
        newhistory->detailLabel->setText("Already voting for " + findd->second->dc[dcindex].name + " in " + findd->second->vp.desc);
        newhistory->dateLabel->setText(QDateTime::currentDateTime().toString("h:mm AP\nddd, M/d/yyyy"));
        QListWidgetItem* item = new QListWidgetItem;
        panelWidget->historyWidget->set_list_theme_vote.emplace(hvp.toStdString(), newhistory);
        panelWidget->historyWidget->set_list_item_vote.emplace(hvp.toStdString(), item);
        item->setSizeHint(newhistory->sizeHint());
        panelWidget->historyWidget->listWidget->insertItem(0,item);
        panelWidget->historyWidget->listWidget->setItemWidget(item, newhistory);
        Information("Voting for " + findd->second->dc[dcindex].name + " in " +findd->second->vp.desc + " success.");
        voteDialog->close();
    }
    return;
}

void TRSVote::handleResult(QString hvp) {
    panelWidget->resultWidget->clearlistWidget();
    panelWidget->frameUsername->hide();
    if (auto findd = set_display.find(hvp.toStdString()); findd != set_display.end()) {
        int totalvote = 0;
        
        for (int a = 0; a < findd->second->dc.size(); a++) {
            totalvote += findd->second->dc[a].totalvote;
        }
        int tvlabel = totalvote;
        panelWidget->resultWidget->moreLabel->setText(findd->second->vp.desc + " has concluded with "+QString::number(tvlabel)+" votes from a total of " + QString::number(findd->second->dp.size()) + " participants.");
        if (totalvote == 0) {
            totalvote++;
        }
        for (int a = 0; a < findd->second->dc.size(); a++) {
            resultItem* theme = new resultItem;
            theme->voteBar->setMaximum(totalvote);
            theme->voteBar->setValue(findd->second->dc[a].totalvote);
            theme->voteLabel->setText(QString::number(findd->second->dc[a].totalvote) + " Votes / " + QString::number(tvlabel) + " Votes");
            theme->nameLabel->setText(findd->second->dc[a].name);
            theme->pkLabel->setText("Publickey : "+findd->second->dc[a].pk);
            QListWidgetItem* item = new QListWidgetItem;
            item->setSizeHint(theme->sizeHint());
            panelWidget->resultWidget->listWidget->addItem(item);
            panelWidget->resultWidget->listWidget->setItemWidget(item, theme);
            panelWidget->resultWidget->listTheme.push_back(theme);
            panelWidget->resultWidget->listItem.push_back(item);
        }
    }
    panelWidget->contentStack->setCurrentWidget(panelWidget->resultWidget);
    panelWidget->backButton->showButton();
}

void TRSVote::setupDialog(QString hvp, int m) {
    if (m == 1) {
        panelWidget->listMemberWidget->clearList();
        panelWidget->frameUsername->hide();
        panelWidget->backButton->showButton();
        int index = panelWidget->listPoll->currentRow();
        std::string key = panelWidget->set_list_hvp[index].c_str();
        auto find = panelWidget->set_list_theme.find(key);
        if (find == panelWidget->set_list_theme.end()) {
            Information("Error while parsing data, try restarting app");
            return;
        }
        panelWidget->listMemberWidget->hvp = hvp;
        QString desc = find->second->pollLabel->text();
        panelWidget->listMemberWidget->titleLabel->setText("List member of "+ desc);
        panelWidget->listMemberWidget->mainStack->setCurrentWidget(panelWidget->listMemberWidget->pleaseWaitFrame);
        panelWidget->contentStack->setCurrentWidget(panelWidget->listMemberWidget);
        _helper->Method_Admin_List(1, hvp);
    }
    else if (m == 2) {
        addDialog->candidateLineEdit->clear();
        addDialog->candidatePkLineEdit->clear();
        addDialog->hvp = hvp;
        addDialog->roleComboBox->setCurrentIndex(0);
        addDialog->exec();
    }
    else if (m == 3) {
        auto find = panelWidget->set_list_theme.find(hvp.toStdString());
        if (find == panelWidget->set_list_theme.end()) {
            Information("Failed to get detail about this polling. Please consider re-login or restart application.");
            panelWidget->button2->setDisabled(true);
            panelWidget->button3->setDisabled(true);
            panelWidget->button4->setDisabled(true);
            return;
        }
        detailDialog->descData->setText(find->second->pollLabel->text());
        detailDialog->pkLineEdit->setText(find->second->pk);
        detailDialog->sigLineEdit->setPlainText(find->second->sig);
        detailDialog->candidateDataLabel->setText("Current : "+ QString::number(find->second->currentcandidate));
        detailDialog->participantDataLabel->setText("Current : " +QString::number(find->second->currentvoter));
        detailDialog->ringDataLabel->setText(QString::number(find->second->ringsize));
        detailDialog->candidateMaxLabel->setText("Maximum : " + QString::number(find->second->maxcandidate));
        detailDialog->participantMaxLabel->setText("Maximum : " + QString::number(find->second->maxvoter));
        detailDialog->blockDateData->setText(QDateTime::fromSecsSinceEpoch(find->second->blocktime).toString("h:mm AP\nddd, M/d/yyyy"));
        detailDialog->startDateData->setText(QDateTime::fromSecsSinceEpoch(find->second->startdate).toString("h:mm AP\nddd, M/d/yyyy"));
        detailDialog->endDateData->setText(QDateTime::fromSecsSinceEpoch(find->second->enddate).toString("h:mm AP\nddd, M/d/yyyy"));
        detailDialog->hvp = hvp;
        detailDialog->exec();
    }
    panelWidget->listPoll->setCurrentRow(-1);
    panelWidget->button2->setDisabled(true);
    panelWidget->button3->setDisabled(true);
    panelWidget->button4->setDisabled(true);
}

void TRSVote::HelperLogin(int a, QVector<QString> data) {
    if (a == 1) {
        panelWidget->usernameDataLabel->setText(storedKey->username.c_str());
        loginWidget->usernameLineEdit->clear();
        loginWidget->passwordLineEdit->clear();
        if (storedKey->role == 1) {
            panelWidget->historyButton->hideButton();
            panelWidget->roleDataLabel->setText("Admin");
            panelWidget->setupPanel(1);
        }
        else if (storedKey->role == 2) {
            panelWidget->historyButton->showButton();
            panelWidget->button3->hide();
            panelWidget->button2->hide();
            panelWidget->button1->hide();
            panelWidget->roleDataLabel->setText("User");
            panelWidget->setupPanel(2);
        }
        loginWidget->endWait();
        stackedWidget->setCurrentWidget(panelWidget);
    }
    else if(a == 2) {
        std::string key = data[0].toStdString();
        if (auto find = panelWidget->set_list_theme.find(key); find != panelWidget->set_list_theme.end()) {
            find->second->startdate = data[2].toLongLong();
            find->second->enddate = data[3].toLongLong();
            find->second->maxcandidate = data[4].toLongLong();
            find->second->maxvoter = data[5].toLongLong();
            find->second->currentcandidate = data[6].toLongLong();
            find->second->currentvoter = data[7].toLongLong();
            find->second->blocktime = data[8].toLongLong();
            find->second->pollLabel->setText(data[1]);
            find->second->ringsize = data[9].toLongLong();
            find->second->pk = data[10];
            find->second->sig = data[11];
            find->second->hashvotepoll = data[0];
            find->second->flagLabel->hide();
            if (storedKey != nullptr) {
                if (storedKey->role == 1) {
                    find->second->mailButton->show();
                    find->second->setLabelColor(1);
                }
                else if (storedKey->role == 2) {
                    find->second->registerButton->show();
                    find->second->setLabelColor(2);
                }
            }
        }
        else {
            ClickableLayoutWidget* newlayout = new ClickableLayoutWidget;
            QString desc = data[1];
            newlayout->hashvotepoll = data[0];
            newlayout->pollLabel->setText(desc);
            newlayout->startdate = data[2].toLongLong();
            newlayout->enddate = data[3].toLongLong();
            newlayout->maxcandidate = data[4].toLongLong();
            newlayout->maxvoter = data[5].toLongLong();
            newlayout->currentcandidate = data[6].toLongLong();
            newlayout->currentvoter = data[7].toLongLong();
            newlayout->blocktime = data[8].toLongLong();
            newlayout->ringsize = data[9].toLongLong();
            newlayout->pk = data[10];
            newlayout->sig = data[11];
            if (storedKey != nullptr) {
                newlayout->setLabelColor(storedKey->role);
                
            }
            QListWidgetItem* newlist = new QListWidgetItem;
            newlist->setSizeHint(newlayout->sizeHint());
            panelWidget->listPoll->insertItem(0, newlist);
            panelWidget->listPoll->setItemWidget(newlist, newlayout);
            panelWidget->set_list_theme.emplace(key, newlayout);
            panelWidget->set_list_item.emplace(key, newlist);
            panelWidget->set_list_hvp.push_front(key);
            connect(newlayout, &ClickableLayoutWidget::registerHVP, this, &TRSVote::handleRegisterWHelper);
            connect(newlayout, &ClickableLayoutWidget::voteHVP, this, &TRSVote::handleVoteWHelper);
            connect(newlayout, &ClickableLayoutWidget::resultHVP, this, &TRSVote::handleResultWHelper);
            connect(newlayout, &ClickableLayoutWidget::mailHVP, this, &TRSVote::handleMailWHelper);
        }
    }
}

void TRSVote::setFlagHelper(int a, std::string hvp, bool s) {
    if (storedKey == nullptr) {
        return;
    }
    if (storedKey->role == 1) {
        return;
    }
    auto find = panelWidget->set_list_theme.find(hvp);
    if (find == panelWidget->set_list_theme.end()) {
        return;
    }
    if (a == 1) {
        
    }
    else if (a == 2) {
        find->second->asCandidateFlag = true;
        find->second->setLabelColor(2);
    }
    else if (a == 3) {
        find->second->nonVFlag = false;
        find->second->setLabelColor(2);
    }
    else if (a == 4) {
        find->second->requestFlag = true;
        find->second->setLabelColor(2);
    }
    else if (a == 5) {
        find->second->voteFlag = true;
        find->second->setLabelColor(2);
    }
}

void TRSVote::handleRegisterWHelper(QString hvp) {
    registerDialog->roleComboBox->setCurrentIndex(0);
    registerDialog->hvp = hvp;
    registerDialog->exec();
}

void TRSVote::handleVoteWHelper(QString hvp) {
    voteDialog->hvp = hvp;
    voteDialog->clearlistWidget();
    QByteArray hash = QByteArray::fromHex(hvp.toStdString().c_str());
    _helper->Method_User_Vote(1, &hash);
}

void TRSVote::handleResultWHelper(QString hvp) {
    panelWidget->resultWidget->totalvote = 0;
    panelWidget->resultWidget->clearlistWidget();
    panelWidget->resultWidget->hvp = hvp;
    _helper->Method_Admin_Result(1, hvp);
}

void TRSVote::handleMailWHelper(QString hvp) {
    auto find = panelWidget->set_list_theme.find(hvp.toStdString());
    if (find == panelWidget->set_list_theme.end()) {
        Information("Error while parsing data, try restarting app");
        return;
    }
    panelWidget->mailWidget->titleLabel->setText("Mail of " + find->second->pollLabel->text());
    panelWidget->mailWidget->hideMail();
    panelWidget->contentStack->setCurrentWidget(panelWidget->mailWidget);
    panelWidget->backButton->showButton();
    panelWidget->frameUsername->hide();
    panelWidget->mailWidget->mainStack->setCurrentWidget(panelWidget->mailWidget->pleaseWaitFrame);
    panelWidget->mailWidget->clearMail();
    panelWidget->mailWidget->hvp = hvp.toStdString();
    uint8_t uhash[32]{};
    Utility::hexstring_to_uint8_t(uhash, 32, hvp.toStdString());
    QByteArray temp = QByteArray((char*)&uhash[0], 32);
    _helper->Method_Admin_Mail(1, &temp);
}

void TRSVote::setHistoryHelper(QString hvp, QString s, long long time, int a) {
    if (a == 1) {
        if (panelWidget->historyWidget->set_list_theme_req.find(hvp.toStdString()) != panelWidget->historyWidget->set_list_theme_req.end()) {
            return;
        }
    }
    else if (a == 2) {
        if (panelWidget->historyWidget->set_list_theme_reg.find(hvp.toStdString()) != panelWidget->historyWidget->set_list_theme_reg.end()) {
            return;
        }
    }
    else if (a == 3) {
        if (panelWidget->historyWidget->set_list_theme_vote.find(hvp.toStdString()) != panelWidget->historyWidget->set_list_theme_vote.end()) {
            return;
        }
    }
    HistoryItem* theme = new HistoryItem;
    theme->dateLabel->setText(QDateTime::fromSecsSinceEpoch(time).toString("h:mm AP\nddd, M/d/yyyy"));
    theme->detailLabel->setText(s);
    QListWidgetItem* item = new QListWidgetItem;
    item->setSizeHint(theme->sizeHint());
    panelWidget->historyWidget->listWidget->insertItem(0, item);
    panelWidget->historyWidget->listWidget->setItemWidget(item, theme);
    if (a == 1) {
        panelWidget->historyWidget->set_list_theme_req.emplace(hvp.toStdString(), theme);
        panelWidget->historyWidget->set_list_item_req.emplace(hvp.toStdString(), item);
    }
    else if (a == 2) {
        panelWidget->historyWidget->set_list_theme_reg.emplace(hvp.toStdString(), theme);
        panelWidget->historyWidget->set_list_item_reg.emplace(hvp.toStdString(), item);
    }
    else if (a == 3) {
        panelWidget->historyWidget->set_list_theme_vote.emplace(hvp.toStdString(), theme);
        panelWidget->historyWidget->set_list_item_vote.emplace(hvp.toStdString(), item);
    }
}

void TRSVote::HelperToMemPoolNewData(QByteArray dat) {
    _mempool->receiveNewData(&dat);
}

void TRSVote::HelperAddMoreSize(std::string hvp, int m) {
    if (auto find = panelWidget->set_list_theme.find(hvp); find != panelWidget->set_list_theme.end()) {
        if (m == 1) {
            find->second->currentcandidate++;
        }
        else if (m == 2) {
            find->second->currentvoter++;
        }
    }
}

void TRSVote::setMailHelper(int a, QString pk, QByteArray cipher, long long blocktime) {
    if (a == 1) {
        int thissize = panelWidget->mailWidget->listWidget->count();
        int sussize = panelWidget->mailWidget->set_list_hvppk.size();
        if (panelWidget->mailWidget->set_list_theme.empty()) {
            panelWidget->mailWidget->listWidget->setDisabled(false);
        }
        if (thissize != sussize) {
            panelWidget->mailWidget->clearMail();
            panelWidget->mailWidget->listWidget->setDisabled(false);
        }
        MessageListWidget* theme = new MessageListWidget;
        theme->blocktime = blocktime;
        theme->cipher = cipher;
        theme->pk = pk;
        theme->hvpstartdate = panelWidget->set_list_theme.find(panelWidget->mailWidget->hvp)->second->startdate;
        theme->desc = panelWidget->set_list_theme.find(panelWidget->mailWidget->hvp)->second->pollLabel->text();
        theme->openCipher(storedKey->key.privatekey);
        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(theme->sizeHint());
        panelWidget->mailWidget->listWidget->insertItem(0, item);
        panelWidget->mailWidget->listWidget->setItemWidget(item, theme);
        panelWidget->mailWidget->set_list_theme.emplace(pk.toStdString(), theme);
        panelWidget->mailWidget->set_list_item.emplace(pk.toStdString(), item);
        panelWidget->mailWidget->set_list_hvppk.push_front(pk.toStdString());
    }
    else if (a == 2) {
        if (panelWidget->mailWidget->set_list_hvppk.isEmpty()) {
            panelWidget->mailWidget->listWidget->addItem("Mail is empty");
            panelWidget->mailWidget->listWidget->setDisabled(true);
        }
        panelWidget->mailWidget->hvp = pk.toStdString();
        panelWidget->mailWidget->mainStack->setCurrentWidget(panelWidget->mailWidget->mainWidget);
    }
    else if (a == 3) {
        panelWidget->mailWidget->acceptMail();
    }
    else if (a == 4) {
        auto find = panelWidget->set_list_theme.find(panelWidget->mailWidget->hvp);
        if (find == panelWidget->set_list_theme.end()) {
            return;
        }
        panelWidget->mailWidget->mailClose(find->second->currentcandidate, find->second->maxcandidate, find->second->currentvoter, find->second->maxvoter);
    }
    else if (a == 5) {
        auto findtheme = panelWidget->mailWidget->set_list_theme.find(pk.toStdString());
        if (findtheme != panelWidget->mailWidget->set_list_theme.end()) {
            findtheme->second->acceptedtime = blocktime;
            findtheme->second->updateAccepted();
        }
        panelWidget->mailWidget->acceptMail();
    }
}

void TRSVote::setListHelper(int con, QVector<QString> data, long long blocktime) {
    if (con == 1) {
        ListMemberWidgetItem* theme = new ListMemberWidgetItem;
        theme->nameLabel->setText(data[0]);
        theme->pkLabel->setText(data[1]);
        theme->dateLabel->setText(QDateTime::fromSecsSinceEpoch(blocktime).toString("h:mm AP\nddd, M/d/yyyy"));
        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(theme->sizeHint());
        panelWidget->listMemberWidget->set_list_theme_candidate.push_front(theme);
        panelWidget->listMemberWidget->set_list_item_candidate.push_front(item);
        panelWidget->listMemberWidget->candidateListWidget->insertItem(0, item);
        panelWidget->listMemberWidget->candidateListWidget->setItemWidget(item, theme);
    }
    else if (con == 2) {
        ListMemberWidgetItem* theme = new ListMemberWidgetItem;
        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(theme->sizeHint());
        theme->pkLabel->setText(data[0]);
        theme->dateLabel->setText(QDateTime::fromSecsSinceEpoch(blocktime).toString("h:mm AP\nddd, M/d/yyyy"));
        theme->nameLabel->hide();
        theme->mainLayout->removeItem(theme->mainLayout->itemAt(1));
        panelWidget->listMemberWidget->set_list_theme_voter.push_front(theme);
        panelWidget->listMemberWidget->set_list_item_voter.push_front(item);
        panelWidget->listMemberWidget->voterListWidget->insertItem(0, item);
        panelWidget->listMemberWidget->voterListWidget->setItemWidget(item, theme);
    }
    else if (con == 3) {
        panelWidget->listMemberWidget->mainStack->setCurrentWidget(panelWidget->listMemberWidget->mainFrame);
        panelWidget->listMemberWidget->searchKeywordCandidateLabel->hide();
        panelWidget->listMemberWidget->refreshCandidateButton->hide();
        panelWidget->listMemberWidget->searchPushButtonCandidate->setDisabled(false);
        panelWidget->listMemberWidget->searchLineEditCandidate->setDisabled(false);
        panelWidget->listMemberWidget->searchComboboxCandidate->setDisabled(false);
        panelWidget->listMemberWidget->headerListWidgetCandidateFrame->setVisible(true);
        panelWidget->listMemberWidget->candidateStackedMain->setCurrentWidget(panelWidget->listMemberWidget->candidateListWidget);
        panelWidget->listMemberWidget->searchKeywordVoterLabel->hide();
        panelWidget->listMemberWidget->refreshVoterButton->hide();
        panelWidget->listMemberWidget->searchPushButtonVoter->setDisabled(false);
        panelWidget->listMemberWidget->searchLineEditVoter->setDisabled(false);
        panelWidget->listMemberWidget->headerListWidgetVoterFrame->setVisible(true);
        panelWidget->listMemberWidget->voterStackedMain->setCurrentWidget(panelWidget->listMemberWidget->voterListWidget);
        if (panelWidget->listMemberWidget->candidateListWidget->count() == 0) {
            panelWidget->listMemberWidget->searchKeywordCandidateLabel->setText("Candidate is empty");
            panelWidget->listMemberWidget->searchKeywordCandidateLabel->show();
            panelWidget->listMemberWidget->refreshCandidateButton->show();
        }
        if (panelWidget->listMemberWidget->voterListWidget->count() == 0) {
            panelWidget->listMemberWidget->searchKeywordVoterLabel->setText("Voter is empty");
            panelWidget->listMemberWidget->searchKeywordVoterLabel->show();
            panelWidget->listMemberWidget->refreshVoterButton->show();
        }
    }
    else if (con == 4) {
        QString text = "Keyword is not found";
        panelWidget->listMemberWidget->searchKeywordCandidateLabel->setText(text);
        panelWidget->listMemberWidget->searchKeywordCandidateLabel->show();
        panelWidget->listMemberWidget->refreshCandidateButton->setDisabled(false);
        panelWidget->listMemberWidget->refreshCandidateButton->show();
        panelWidget->listMemberWidget->searchPushButtonCandidate->setDisabled(false);
        panelWidget->listMemberWidget->searchLineEditCandidate->setDisabled(false);
        panelWidget->listMemberWidget->searchComboboxCandidate->setDisabled(false);
        panelWidget->listMemberWidget->headerListWidgetCandidateFrame->setVisible(true);
        panelWidget->listMemberWidget->candidateStackedMain->setCurrentWidget(panelWidget->listMemberWidget->candidateListWidget);
    }
    else if (con == 5) {
        QString text = "Keyword is not found";
        panelWidget->listMemberWidget->searchKeywordVoterLabel->setText(text);
        panelWidget->listMemberWidget->searchKeywordVoterLabel->show();
        panelWidget->listMemberWidget->refreshVoterButton->setDisabled(false);
        panelWidget->listMemberWidget->refreshVoterButton->show();
        panelWidget->listMemberWidget->searchPushButtonVoter->setDisabled(false);
        panelWidget->listMemberWidget->searchLineEditVoter->setDisabled(false);
        panelWidget->listMemberWidget->headerListWidgetVoterFrame->setVisible(true);
        panelWidget->listMemberWidget->voterStackedMain->setCurrentWidget(panelWidget->listMemberWidget->voterListWidget);
    }
    else if (con == 6) {
        QString text = QString::number(panelWidget->listMemberWidget->candidateListWidget->count()) + " out of " + QString::number(blocktime) + " " + panelWidget->listMemberWidget->searchKeywordCandidateLabel->text();
        panelWidget->listMemberWidget->searchKeywordCandidateLabel->setText(text);
        panelWidget->listMemberWidget->refreshCandidateButton->setDisabled(false);
        panelWidget->listMemberWidget->searchKeywordCandidateLabel->show();
        panelWidget->listMemberWidget->refreshCandidateButton->show();
        panelWidget->listMemberWidget->searchPushButtonCandidate->setDisabled(false);
        panelWidget->listMemberWidget->searchLineEditCandidate->setDisabled(false);
        panelWidget->listMemberWidget->searchComboboxCandidate->setDisabled(false);
        panelWidget->listMemberWidget->headerListWidgetCandidateFrame->setVisible(true);
        panelWidget->listMemberWidget->candidateStackedMain->setCurrentWidget(panelWidget->listMemberWidget->candidateListWidget);
    }
    else if (con == 7) {
        QString text = QString::number(panelWidget->listMemberWidget->voterListWidget->count()) +" out of " + QString::number(blocktime) + " " + panelWidget->listMemberWidget->searchKeywordVoterLabel->text();
        panelWidget->listMemberWidget->searchKeywordVoterLabel->setText(text);
        panelWidget->listMemberWidget->refreshVoterButton->setDisabled(false);
        panelWidget->listMemberWidget->searchKeywordVoterLabel->show();
        panelWidget->listMemberWidget->refreshVoterButton->show();
        panelWidget->listMemberWidget->searchPushButtonVoter->setDisabled(false);
        panelWidget->listMemberWidget->searchLineEditVoter->setDisabled(false);
        panelWidget->listMemberWidget->headerListWidgetVoterFrame->setVisible(true);
        panelWidget->listMemberWidget->voterStackedMain->setCurrentWidget(panelWidget->listMemberWidget->voterListWidget);
    }
    else if (con == 8) {
        panelWidget->listMemberWidget->searchKeywordCandidateLabel->hide();
        panelWidget->listMemberWidget->refreshCandidateButton->hide();
        panelWidget->listMemberWidget->searchPushButtonCandidate->setDisabled(false);
        panelWidget->listMemberWidget->searchLineEditCandidate->setDisabled(false);
        panelWidget->listMemberWidget->searchComboboxCandidate->setDisabled(false);
        panelWidget->listMemberWidget->headerListWidgetCandidateFrame->setVisible(true);
        panelWidget->listMemberWidget->candidateStackedMain->setCurrentWidget(panelWidget->listMemberWidget->candidateListWidget);
    }
    else if (con == 9) {
        panelWidget->listMemberWidget->searchKeywordVoterLabel->hide();
        panelWidget->listMemberWidget->refreshVoterButton->hide();
        panelWidget->listMemberWidget->searchPushButtonVoter->setDisabled(false);
        panelWidget->listMemberWidget->searchLineEditVoter->setDisabled(false);
        panelWidget->listMemberWidget->headerListWidgetVoterFrame->setVisible(true);
        panelWidget->listMemberWidget->voterStackedMain->setCurrentWidget(panelWidget->listMemberWidget->voterListWidget);
    }
}

void TRSVote::setResultHelper(int a, VoteTree::DataCandidateString dat) {
    if (a == 1) {
        auto find = panelWidget->set_list_theme.find(panelWidget->resultWidget->hvp.toStdString());
        if (find == panelWidget->set_list_theme.end()) {
            return;
        }
        int max = find->second->currentvoter;
        resultItem* theme = new resultItem;
        theme->voteLabel->setText(QString::number(dat.totalvote));
        theme->nameLabel->setText(dat.name);
        theme->pkLabel->setText(dat.pk);
        theme->voteBar->setMaximum(max);
        theme->voteBar->setValue(dat.totalvote);
        panelWidget->resultWidget->totalvote += dat.totalvote;
        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(theme->sizeHint());
        panelWidget->resultWidget->listWidget->addItem(item);
        panelWidget->resultWidget->listWidget->setItemWidget(item, theme);
        panelWidget->resultWidget->listTheme.push_back(theme);
        panelWidget->resultWidget->listItem.push_back(item);
    }
    else if (a == 2) {
        auto find = panelWidget->set_list_theme.find(panelWidget->resultWidget->hvp.toStdString());
        if (find == panelWidget->set_list_theme.end()) {
            Information("Error while parsing data, try restarting app");
            return;
        }
        int max = panelWidget->resultWidget->totalvote;
        int label = max;
        panelWidget->resultWidget->moreLabel->setText(find->second->pollLabel->text() + " has concluded with "+QString::number(max) + " votes from a total of "+QString::number(find->second->currentvoter) + " participants");
        if (max == 0) {
            max++;
        }
        for (int it = 0; it < panelWidget->resultWidget->listTheme.size(); it++) {
            QString text = panelWidget->resultWidget->listTheme[it]->voteLabel->text() + " Votes / " + QString::number(label)+ " Votes";
            panelWidget->resultWidget->listTheme[it]->voteLabel->setText(text);
            panelWidget->resultWidget->listTheme[it]->voteBar->setMaximum(max);
        }
        panelWidget->backButton->showButton();
        panelWidget->frameUsername->hide();
        panelWidget->contentStack->setCurrentWidget(panelWidget->resultWidget);
    }
}

void TRSVote::setVoteHelper(int a, VoteTree::DataCandidateString dat) {
    if (a == 1) {
        auto find = panelWidget->set_list_theme.find(voteDialog->hvp.toStdString());
        if (find == panelWidget->set_list_theme.end()) {
            return;
        }
        voteItem* theme = new voteItem;
        theme->pk = dat.pk;
        theme->hvp = voteDialog->hvp;
        theme->pkLabel->setText(theme->pk);
        theme->nameLabel->setText(dat.name);
        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(theme->sizeHint());
        voteDialog->listWidget->addItem(item);
        voteDialog->listWidget->setItemWidget(item, theme);
        voteDialog->listTheme.push_back(theme);
        voteDialog->listItem.push_back(item);
        connect(theme, &voteItem::doVote, this, &TRSVote::doingVoteWHelper);
    }
    else if (a == 2) {
        auto find = panelWidget->set_list_theme.find(voteDialog->hvp.toStdString());
        if (find == panelWidget->set_list_theme.end()) {
            Information("Error while parsing data, try restarting app");
            return;
        }
        voteDialog->titleLabel->setText("Ballot " + find->second->pollLabel->text());
        voteDialog->exec();
    }
}

void TRSVote::doingVoteWHelper(QString hvp, QString pk) {
    QDialog dialog;
    QMessageBox warningBox(QMessageBox::Information, "TRS Vote", "Are you sure want to choose candidate A in Vote Poll B?", QMessageBox::Yes | QMessageBox::No, &dialog);
    warningBox.setStyleSheet("background-color: #F5F8FA; border: none; border-radius: 5px;");
    warningBox.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    // Retrieve the buttons
    QAbstractButton* yesButton = warningBox.button(QMessageBox::Yes);
    QAbstractButton* noButton = warningBox.button(QMessageBox::No);

    // Modify the buttons
    yesButton->setText("Agree");
    yesButton->setStyleSheet("QPushButton {"
        "background-color: #1DA1F2;"
        "border-radius: 20px;"
        "color: #FFFFFF;"
        "padding: 5px 20px;"
        "}"
        "QPushButton:hover {"
        "background-color: #0F8BDD;"
        "}"
        "QPushButton:pressed {"
        "background-color: #0A5F95;"
        "}");

    noButton->setText("Disagree");
    noButton->setStyleSheet("QPushButton {"
        "background-color: #F44336;"
        "border-radius: 20px;"
        "color: #FFFFFF;"
        "padding: 5px 20px;"
        "}"
        "QPushButton:hover {"
        "background-color: #D32F2F;"
        "}"
        "QPushButton:pressed {"
        "background-color: #B71C1C;"
        "}");

    int response = warningBox.exec();

    if (response == QMessageBox::Yes) {
        QByteArray dat = QByteArray::fromHex(hvp.toStdString().c_str());
        dat.append(QByteArray::fromHex(pk.toStdString().c_str()));
        _helper->Method_User_Vote(2, &dat);
        voteDialog->close();
    }
    else {
        voteDialog->close();
    }
}

void TRSVote::removePeer(long long key) {
    nsdialog->removePeer(key);
}

void TRSVote::Update_BlockHeight(int c) {
    blockheightlabel->setText("Block Height : " + QString::number(c));
}

void TRSVote::parseBlockHelper(QString path) {
    _helper->Method_parseBlock(path);
}

void TRSVote::parseBlockFromBegHelper(QVector<QString> path) {
    _helper->Method_parseBlockFromBeg(path);
}