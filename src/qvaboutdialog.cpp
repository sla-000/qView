#include "qvaboutdialog.h"
#include "ui_qvaboutdialog.h"
#include <QFontDatabase>
#include <QJsonDocument>

QVAboutDialog::QVAboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QVAboutDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::CustomizeWindowHint);
    setWindowModality(Qt::ApplicationModal);

    // add fonts
    QFontDatabase::addApplicationFont(":/fonts/resources/Lato-Light.ttf");
    QFontDatabase::addApplicationFont(":/fonts/resources/Lato-Regular.ttf");

    int modifier = 0;
    //set main title font
#ifdef Q_OS_MACOS
    const QFont font1 = QFont("Lato", 96, QFont::Light);
    modifier = 4;
#else
    const QFont font1 = QFont("Lato", 72, QFont::Light);
#endif
    ui->logoLabel->setFont(font1);

    //set subtitle font & text
    QFont font2 = QFont("Lato", 18 + modifier);
    font2.setStyleName("Regular");
    QString subtitleText = tr("version ") + QString::number(VERSION, 'f', 1);
    // If this is a nightly build, display the build number
#ifdef NIGHTLY
        subtitleText = tr("🌒 Nightly");
#endif
    ui->subtitleLabel->setFont(font2);
    ui->subtitleLabel->setText(subtitleText);

    //set update font & text
    QFont font3 = QFont("Lato", 10 + modifier);
    font3.setStyleName("Regular");
    const QString updateText = tr("Checking for updates...");
    ui->updateLabel->setFont(font3);
    ui->updateLabel->setText(updateText);
    ui->updateLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->updateLabel->setOpenExternalLinks(true);

    //set infolabel2 font, text, and properties
    QFont font4 = QFont("Lato", 8 + modifier);
    font4.setStyleName("Regular");
    const QString labelText2 = tr("Built with Qt %1<br>Source code available under GPLv3 on <a style=\"color: #03A9F4; text-decoration:none;\" href=\"https://github.com/jurplel/qView\">Github</a><br>Icon glyph created by Guilhem from the Noun Project<br>Copyright © 2018-2020 jurplel and qView contributors").arg(QT_VERSION_STR);
    ui->infoLabel2->setFont(font4);
    ui->infoLabel2->setText(labelText2);

    ui->infoLabel2->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->infoLabel2->setOpenExternalLinks(true);

    requestUpdates();
}

QVAboutDialog::~QVAboutDialog()
{
    delete ui;
}

void QVAboutDialog::requestUpdates()
{
    // Don't check for updates on nightly builds
#ifdef NIGHTLY
    #define STRINGIFY2(s) #s
    #define STRINGIFY(s) STRINGIFY2(s)
    QString text = tr("Build number #") + STRINGIFY(NIGHTLY);
    ui->updateLabel->setText(text);
    return;
#endif

    // send network request for update check
    QUrl url("https://api.github.com/repos/jurplel/qview/releases");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    auto *networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(checkUpdates(QNetworkReply*)));

    networkManager->get(request);
}

void QVAboutDialog::checkUpdates(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        ui->updateLabel->setText(tr("Error checking for updates"));
        return;
    }

    QByteArray byteArray = reply->readAll();
    QJsonDocument json = QJsonDocument::fromJson(byteArray);

    if (json.isNull())
    {
        ui->updateLabel->setText(tr("Error checking for updates"));
        return;
    }

    double latestVersionNum = json.array().first().toObject().value("tag_name").toString("0.0").toDouble();

    if (latestVersionNum == 0.0)
    {
        ui->updateLabel->setText(tr("Error checking for updates"));
        return;
    }

    if (latestVersionNum > VERSION)
    {
        const QString text = (R"(<a style="color: #03A9F4; text-decoration:none;" href="https://github.com/jurplel/qView/releases">)" + tr("%1 update available!") + "</a>").arg(QString::number(latestVersionNum, 'f', 1));
        ui->updateLabel->setText(text);
    }
    else
    {
        ui->updateLabel->setText(tr("No updates available"));
    }
}
