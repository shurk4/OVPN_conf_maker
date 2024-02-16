#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QSettings>
#include <QClipboard>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QSet>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString getCRACreateCommand(const QString userName);

    void readSettings();
    void writeSettings();

    QString selectDir(const QString dir);

    void searchClients();
    QString getData(const QString &path, const QString &clientName, const QString &ext);
    QString getCert(const QString &path, const QString &clientName);
    QString getConfig(const QString &clientName);
    void makeConfigFile(const QString &path, const QString &name, const QString &config);

private slots:
    void on_pushButtonToClipBoard_clicked();

    void on_pushButtonCommandGen_clicked();

    void on_pushButtonSelectCertsFolder_clicked();

    void on_pushButtonStartActions_clicked();

    void on_pushButtonSelectConfDir_clicked();

private:
    Ui::MainWindow *ui;

    bool isCommandGenerated = false;
    QString certsDir;
    QString confDir;
    QSet<QString> clientsList;
    };
#endif // MAINWINDOW_H
