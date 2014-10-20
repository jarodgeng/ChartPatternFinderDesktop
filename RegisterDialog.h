#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class RegisterDialog : public QDialog
{
    Q_OBJECT
private:
    QLineEdit *licenseKeyEntry_;
    QLineEdit *emailEntry_;
    QPushButton *registerButton_;
    QPushButton *cancelButton_;

    void enableRegistration();
    void disableRegistration();
public:
    explicit RegisterDialog(QWidget *parent = 0);

signals:

public slots:
private slots:
    void emailTextChanged(const QString &);
    void licenseTextChanged(const QString &);
    void registerButtonClicked();
    void cancelButtonClicked();


};

#endif // REGISTERDIALOG_H