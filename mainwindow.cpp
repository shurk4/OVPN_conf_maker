#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    readSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

QString MainWindow::getCRACreateCommand(const QString userName)
{
    return "easyrsa build-client-full " + userName + " nopass\nopenvpn --tls-crypt-v2 ${EASYRSA_PKI}/private/server.pem \\\n--genkey tls-crypt-v2-client ${EASYRSA_PKI}/private/" + userName + ".pem";
}

void MainWindow::readSettings()
{
    QSettings settings("ShurkSoft", "OVPN_conf_maker");
    settings.beginGroup("clientSettings");
    ui->lineEditRemote->setText(settings.value("remote").toString());
    ui->lineEditPort->setText(settings.value("port").toString());
    ui->lineEditUser->setText(settings.value("user").toString());
    ui->lineEditGroup->setText(settings.value("group").toString());
    ui->comboBoxDev->setCurrentIndex(settings.value("dev").toInt());
    ui->textEditAnotherParms->setText(settings.value("extras").toString());
    certsDir = settings.value("certsDir").toString();
    if(!certsDir.isEmpty()) ui->labelCertsDir->setText(certsDir);
    confDir = settings.value("confDir").toString();
    if(!confDir.isEmpty()) ui->labelConfDir->setText(confDir);
    settings.endGroup();
}

void MainWindow::writeSettings()
{
    QSettings settings("ShurkSoft", "OVPN_conf_maker");
    settings.beginGroup("clientSettings");
    settings.setValue("remote", ui->lineEditRemote->text());
    settings.setValue("port", ui->lineEditPort->text());
    settings.setValue("user", ui->lineEditUser->text());
    settings.setValue("group", ui->lineEditGroup->text());
    settings.setValue("dev", ui->comboBoxDev->currentIndex());
    settings.setValue("extras", ui->textEditAnotherParms->toPlainText());
    settings.setValue("certsDir", certsDir);
    settings.setValue("confDir", confDir);
    settings.endGroup();
}

QString MainWindow::selectDir(const QString dir)
{
    QString resultDir = QFileDialog::getExistingDirectory(this, "Select the directory", dir);
    if(resultDir.isEmpty()) return "";
    return resultDir;
}

void MainWindow::searchClients()
{
    QFileInfoList fileList = QDir(certsDir + "/private").entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for(const auto &i : fileList)
    {
        if(i.isFile())
        {
            if(i.baseName() != "ca" && i.baseName() != "server" && !clientsList.contains(i.baseName()))
            {
                clientsList.insert(i.baseName());
            }
        }
    }

    qDebug() << "Founded clients";
    for(const auto &i : qAsConst(clientsList))
    {
        qDebug() << i;
    }
}

QString MainWindow::getData(const QString &path, const QString &clientName, const QString &ext)
{
    QString filePath = path + "/" + clientName + "." + ext;
    qDebug() << "Read file: " << filePath;
    QFile readingFile(filePath);
    QString data;
    if(readingFile.open(QIODevice::ReadOnly))
    {
        while(!readingFile.atEnd())
        {
            QString line = readingFile.readLine();
            data += line;
        }
        readingFile.close();
    }
    else
    {
        qDebug() << "Can't open file";
    }

    if(data[data.size() - 1] == "\n") data.chop(1);
    return data;
}

QString MainWindow::getCert(const QString &path, const QString &clientName)
{
    QString filePath = path + "/" + clientName + "." + "crt";
    qDebug() << "Read file: " << filePath;
    QFile readingFile(filePath);
    QString data;

    if(readingFile.open(QIODevice::ReadOnly))
    {
        bool certLines = false;
        while(!readingFile.atEnd())
        {
            QString line = readingFile.readLine();
            if(certLines || line.contains("-----BEGIN CERTIFICATE-----"))
            {
                certLines = true;
                data += line;
            }
            if(line == "-----END CERTIFICATE-----") break;
        }
        readingFile.close();
    }
    else
    {
        qDebug() << "Can't open file";
    }

    if(data[data.size() - 1] == "\n") data.chop(1);
    return data;
}

QString MainWindow::getConfig(const QString &clientName)
{
    QString config = "";

    config += "#client name: " + clientName + "\n"
        + "user " + ui->lineEditUser->text() + "\n"
        + "group " + ui->lineEditGroup->text() + "\n"
        + "client" + "\n"
        + "remote " + ui->lineEditRemote->text() + " " + ui->lineEditPort->text() + " " + ui->comboBoxProto->currentText() + "\n"
        + "dev " + ui->comboBoxDev->currentText() + "\n"
        + ui->textEditAnotherParms->toPlainText() + "\n"
        + "\n"
        + "<tls-crypt-v2>" + "\n"
        + getData(certsDir + "/private", clientName, "pem") + "\n"
        + "</tls-crypt-v2>" + "\n"
        + "\n"
        + "<key>" + "\n"
        + getData(certsDir + "/private", clientName, "key") + "\n"
        + "</key>" + "\n"
        + "\n"
        + "<cert>" + "\n"
        + getCert(certsDir + "/issued", clientName) + "\n"
        + "</cert>" + "\n"
        + "\n"
        + "<ca>" + "\n"
        + getData(certsDir, "ca", "crt") + "\n"
        + "</ca>";

    return config;
}

void MainWindow::makeConfigFile(const QString &path, const QString &name, const QString &config)
{
    QString filename = path + "/" + name + ".ovpn";
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << config;
        file.close();
    }
    else
    {
        qDebug() << "Failed to create a file";
    }
}

void MainWindow::on_pushButtonToClipBoard_clicked()
{
    if(isCommandGenerated)
    {
        QApplication::clipboard()->setText(ui->textBrowserCRACommand->toPlainText());
        ui->statusBar->showMessage("The command has been copied");
    }
}

void MainWindow::on_pushButtonCommandGen_clicked()
{
    if(ui->lineEditUserName->text().isEmpty())
    {
        return;
    }

    ui->textBrowserCRACommand->setText(getCRACreateCommand(ui->lineEditUserName->text()));
    isCommandGenerated = true;
}

void MainWindow::on_pushButtonSelectCertsFolder_clicked()
{
    certsDir = selectDir(certsDir);
    if(certsDir.isEmpty()) return;
    ui->labelCertsDir->setText(certsDir);
}

void MainWindow::on_pushButtonSelectConfDir_clicked()
{
    confDir = selectDir(confDir);
    if(confDir.isEmpty()) return;
    ui->labelConfDir->setText(confDir);
}

void MainWindow::on_pushButtonStartActions_clicked()
{
    searchClients();

    for(const auto &i : qAsConst(clientsList))
    {
        QString config = getConfig(i);
        makeConfigFile(ui->labelConfDir->text(), i, config);
    }
    ui->statusBar->showMessage("The creation of configuration files is completed");
}

