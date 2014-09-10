/**************************************************************************
**
** Copyright (C) 2012-2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Installer Framework.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
**************************************************************************/

#ifndef PACKAGEMANAGERGUI_H
#define PACKAGEMANAGERGUI_H

#include "packagemanagercore.h"

#include <QtCore/QEvent>
#include <QtCore/QMetaType>

#include <QWizard>
#include <QWizardPage>

// FIXME: move to private classes
QT_BEGIN_NAMESPACE
class QAbstractButton;
class QCheckBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QProgressBar;
class QRadioButton;
class QTextBrowser;
class QWinTaskbarButton;
QT_END_NAMESPACE

namespace QInstaller {

class PackageManagerCore;
class PackageManagerPage;
class PerformInstallationForm;


// -- PackageManagerGui

class INSTALLER_EXPORT PackageManagerGui : public QWizard
{
    Q_OBJECT

public:
    explicit PackageManagerGui(PackageManagerCore *core, QWidget *parent = 0);
    virtual ~PackageManagerGui();
    virtual void init() = 0;

    void loadControlScript(const QString& scriptPath);
    void callControlScriptMethod(const QString& methodName);

    Q_INVOKABLE QWidget *pageById(int id) const;
    Q_INVOKABLE QWidget *pageByObjectName(const QString &name) const;

    Q_INVOKABLE QWidget* currentPageWidget() const;
    Q_INVOKABLE QWidget* pageWidgetByObjectName(const QString &name) const;

    Q_INVOKABLE QString defaultButtonText(int wizardButton) const;
    Q_INVOKABLE void clickButton(int wizardButton, int delayInMs = 0);
    Q_INVOKABLE bool isButtonEnabled(int wizardButton);

    Q_INVOKABLE void showSettingsButton(bool show);
    Q_INVOKABLE void setSettingsButtonEnabled(bool enable);

    void updateButtonLayout();
    static QWizard::WizardStyle getStyle(const QString &name);

Q_SIGNALS:
    void interrupted();
    void languageChanged();
    void finishButtonClicked();
    void gotRestarted();
    void settingsButtonClicked();

public Q_SLOTS:
    void cancelButtonClicked();
    void reject();
    void rejectWithoutPrompt();
    void showFinishedPage();
    void setModified(bool value);

protected Q_SLOTS:
    void wizardPageInsertionRequested(QWidget *widget, QInstaller::PackageManagerCore::WizardPage page);
    void wizardPageRemovalRequested(QWidget *widget);
    void wizardWidgetInsertionRequested(QWidget *widget, QInstaller::PackageManagerCore::WizardPage page);
    void wizardWidgetRemovalRequested(QWidget *widget);
    void wizardPageVisibilityChangeRequested(bool visible, int page);
    void executeControlScript(int pageId);
    void setValidatorForCustomPageRequested(QInstaller::Component *component, const QString &name,
                                            const QString &callbackName);

    void setAutomatedPageSwitchEnabled(bool request);

private Q_SLOTS:
    void onLanguageChanged();
    void customButtonClicked(int which);
    void dependsOnLocalInstallerBinary();

protected:
    bool event(QEvent *event);
    void showEvent(QShowEvent *event);
    PackageManagerCore *packageManagerCore() const { return m_core; }

private:
    class Private;
    Private *const d;
    PackageManagerCore *m_core;
};


// -- PackageManagerPage

class INSTALLER_EXPORT PackageManagerPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PackageManagerPage(PackageManagerCore *core);
    virtual ~PackageManagerPage() {}

    virtual QPixmap logoPixmap() const;
    virtual QString productName() const;
    virtual QPixmap watermarkPixmap() const;
    virtual QPixmap bannerPixmap() const;

    void setColoredTitle(const QString &title);
    void setColoredSubTitle(const QString &subTitle);

    virtual bool isComplete() const;
    void setComplete(bool complete);

    virtual bool isInterruptible() const { return false; }
    PackageManagerGui* gui() const { return qobject_cast<PackageManagerGui*>(wizard()); }

    void setValidatePageComponent(QInstaller::Component *component);

    bool validatePage();

    bool settingsButtonRequested() const { return m_needsSettingsButton; }
    void setSettingsButtonRequested(bool request) { m_needsSettingsButton = request; }

signals:
    void entered();
    void left();

protected:
    PackageManagerCore *packageManagerCore() const;

    // Inserts widget into the same layout like a sibling identified
    // by its name. Default position is just behind the sibling.
    virtual void insertWidget(QWidget *widget, const QString &siblingName, int offset = 1);
    virtual QWidget *findWidget(const QString &objectName) const;

    virtual void setVisible(bool visible); // reimp
    virtual int nextId() const; // reimp

    virtual void entering() {} // called on entering
    virtual void leaving() {}  // called on leaving

    bool isConstructing() const { return m_fresh; }

private:
    bool m_fresh;
    bool m_complete;
    QString m_titleColor;
    bool m_needsSettingsButton;

    PackageManagerCore *m_core;
    QInstaller::Component *validatorComponent;
};


// -- IntroductionPage

class INSTALLER_EXPORT IntroductionPage : public PackageManagerPage
{
    Q_OBJECT

public:
    explicit IntroductionPage(PackageManagerCore *core);

    void setText(const QString &text);

    int nextId() const;
    bool validatePage();

    void showAll();
    void hideAll();
    void showMetaInfoUdate();
    void showMaintenanceTools();
    void setMaintenanceToolsEnabled(bool enable);

    public Q_SLOTS:
    void onCoreNetworkSettingsChanged();
    void setMessage(const QString &msg);
    void onProgressChanged(int progress);
    void setErrorMessage(const QString &error);

Q_SIGNALS:
    void packageManagerCoreTypeChanged();

private Q_SLOTS:
    void setUpdater(bool value);
    void setUninstaller(bool value);
    void setPackageManager(bool value);

private:
    void entering();
    void leaving();

    void showWidgets(bool show);
    void callControlScript(const QString &callback);

    bool validRepositoriesAvailable() const;

private:
    bool m_updatesFetched;
    bool m_allPackagesFetched;

    QLabel *m_label;
    QLabel *m_msgLabel;
    QLabel *m_errorLabel;
    QProgressBar *m_progressBar;
    QRadioButton *m_packageManager;
    QRadioButton *m_updateComponents;
    QRadioButton *m_removeAllComponents;

#ifdef Q_OS_WIN
    QWinTaskbarButton *m_taskButton;
#endif
};


// -- LicenseAgreementPage

class INSTALLER_EXPORT LicenseAgreementPage : public PackageManagerPage
{
    Q_OBJECT
    class ClickForwarder;

public:
    explicit LicenseAgreementPage(PackageManagerCore *core);

    void entering();
    bool isComplete() const;

private Q_SLOTS:
    void openLicenseUrl(const QUrl &url);
    void currentItemChanged(QListWidgetItem *current);

private:
    void addLicenseItem(const QHash<QString, QPair<QString, QString> > &hash);
    void updateUi();

private:
    QTextBrowser *m_textBrowser;
    QListWidget *m_licenseListWidget;

    QRadioButton *m_acceptRadioButton;
    QRadioButton *m_rejectRadioButton;

    QLabel *m_acceptLabel;
    QLabel *m_rejectLabel;
};


// -- ComponentSelectionPage

class INSTALLER_EXPORT ComponentSelectionPage : public PackageManagerPage
{
    Q_OBJECT

public:
    explicit ComponentSelectionPage(PackageManagerCore *core);
    ~ComponentSelectionPage();

    bool isComplete() const;

    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void deselectAll();
    Q_INVOKABLE void selectDefault();
    Q_INVOKABLE void selectComponent(const QString &id);
    Q_INVOKABLE void deselectComponent(const QString &id);

protected:
    void entering();
    void showEvent(QShowEvent *event);

private Q_SLOTS:
    void setModified(bool modified);

private:
    class Private;
    Private *d;
};


// -- TargetDirectoryPage

class INSTALLER_EXPORT TargetDirectoryPage : public PackageManagerPage
{
    Q_OBJECT

public:
    explicit TargetDirectoryPage(PackageManagerCore *core);
    QString targetDir() const;
    void setTargetDir(const QString &dirName);

    void initializePage();
    bool validatePage();
    bool isComplete() const;

protected:
    void entering();
    void leaving();

private Q_SLOTS:
    void dirRequested();

private:
    QString targetDirWarning() const;
    bool askQuestion(const QString &identifier, const QString &message);
    bool failWithError(const QString &identifier, const QString &message);

private:
    QLineEdit *m_lineEdit;
    QLabel *m_warningLabel;
};


// -- StartMenuDirectoryPage

class INSTALLER_EXPORT StartMenuDirectoryPage : public PackageManagerPage
{
    Q_OBJECT

public:
    explicit StartMenuDirectoryPage(PackageManagerCore *core);

    QString startMenuDir() const;
    void setStartMenuDir(const QString &startMenuDir);

protected:
    void leaving();

private Q_SLOTS:
    void currentItemChanged(QListWidgetItem* current);

private:
    QString startMenuPath;
    QLineEdit *m_lineEdit;
    QListWidget *m_listWidget;
};


// -- ReadyForInstallationPage

class INSTALLER_EXPORT ReadyForInstallationPage : public PackageManagerPage
{
    Q_OBJECT

public:
    explicit ReadyForInstallationPage(PackageManagerCore *core);

protected:
    void entering();
    void leaving();

private:
    bool calculateComponents(QString *displayString);

private:
    QLabel *m_msgLabel;
    QTextBrowser* m_taskDetailsBrowser;
};


// -- PerformInstallationPage

class INSTALLER_EXPORT PerformInstallationPage : public PackageManagerPage
{
    Q_OBJECT

public:
    explicit PerformInstallationPage(PackageManagerCore *core);
    ~PerformInstallationPage();
    bool isAutoSwitching() const;

protected:
    void entering();
    void leaving();
    bool isInterruptible() const { return true; }

public Q_SLOTS:
    void setTitleMessage(const QString& title);

Q_SIGNALS:
    void installationRequested();
    void setAutomatedPageSwitchEnabled(bool request);

private Q_SLOTS:
    void installationStarted();
    void installationFinished();

    void uninstallationStarted();
    void uninstallationFinished();

    void toggleDetailsWereChanged();

private:
    PerformInstallationForm *m_performInstallationForm;
};


// -- FinishedPage

class INSTALLER_EXPORT FinishedPage : public PackageManagerPage
{
    Q_OBJECT

public:
    explicit FinishedPage(PackageManagerCore *core);

public Q_SLOTS:
    void handleFinishClicked();
    void cleanupChangedConnects();

protected:
    void entering();
    void leaving();

private:
    QLabel *m_msgLabel;
    QCheckBox *m_runItCheckBox;
    QAbstractButton *m_commitButton;
};


// -- RestartPage

class INSTALLER_EXPORT RestartPage : public PackageManagerPage
{
    Q_OBJECT

public:
    explicit RestartPage(PackageManagerCore *core);

    virtual int nextId() const;

protected:
    void entering();
    void leaving();

Q_SIGNALS:
    void restart();
};

} //namespace QInstaller

#endif  // PACKAGEMANAGERGUI_H
