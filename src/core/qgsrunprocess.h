/***************************************************************************
                          qgsrunprocess.h

 A class that runs an external program

                             -------------------
    begin                : Jan 2005
    copyright            : (C) 2005 by Gavin Macaulay
    email                : gavin at macaulay dot co dot nz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRUNPROCESS_H
#define QGSRUNPROCESS_H

#include <QObject>
#include <QProcess>
#include <QThread>

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsFeedback;
class QgsMessageOutput;

/**
 * \ingroup core
 * A class that executes an external program/script.
 * It can optionally capture the standard output and error from the
 * process and displays them in a dialog box.
 *
 * On some platforms (e.g. iOS) , the process execution is skipped
 * https://lists.qt-project.org/pipermail/development/2015-July/022205.html
 */
class CORE_EXPORT QgsRunProcess: public QObject SIP_NODEFAULTCTORS
{
    Q_OBJECT

  public:
    // This class deletes itself, so to ensure that it is only created
    // using new, the Named Consturctor Idiom is used, and one needs to
    // use the create() static function to get an instance of this class.

    // The action argument contains string with the command.
    // If capture is true, the standard output and error from the process
    // will be sent to QgsMessageOutput - usually a dialog box.
    static QgsRunProcess *create( const QString &action, bool capture ) SIP_FACTORY
    { return new QgsRunProcess( action, capture ); }

  private:
    QgsRunProcess( const QString &action, bool capture ) SIP_FORCE;
    ~QgsRunProcess() override SIP_FORCE;

#if QT_CONFIG(process)
    // Deletes the instance of the class
    void die();

    QProcess *mProcess = nullptr;
    QgsMessageOutput *mOutput = nullptr;
    QString mCommand;

  public slots:
    void stdoutAvailable();
    void stderrAvailable();
    void processError( QProcess::ProcessError );
    void processExit( int, QProcess::ExitStatus );
    void dialogGone();
#endif // !(QT_CONFIG(process)
};

class CORE_EXPORT QgsBlockingProcess : public QObject
{
    Q_OBJECT

  public:

    QgsBlockingProcess( const QString &process, const QStringList &arguments );

#ifndef SIP_RUN
    void setStdOutHandler( const std::function< void( const QByteArray & ) > &handler ) { mStdoutHandler = handler; }
#else
    void setStdOutHandler( SIP_PYCALLABLE / AllowNone / );
    % MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setStdOutHandler( [a0]( const QByteArray &arg )
    {
      SIP_BLOCK_THREADS
      Py_XDECREF( sipCallMethod( NULL, a0, "D", &arg, sipType_QByteArray, NULL ) );
      SIP_UNBLOCK_THREADS
    } );

    Py_END_ALLOW_THREADS
    % End
#endif

#ifndef SIP_RUN
    void setStdErrHandler( const std::function< void( const QByteArray & ) > &handler ) { mStderrHandler = handler; }
#else
    void setStdErrHandler( SIP_PYCALLABLE / AllowNone / );
    % MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setStdErrHandler( [a0]( const QByteArray &arg )
    {
      SIP_BLOCK_THREADS
      Py_XDECREF( sipCallMethod( NULL, a0, "D", &arg, sipType_QByteArray, NULL ) );
      SIP_UNBLOCK_THREADS
    } );

    Py_END_ALLOW_THREADS
    % End
#endif

    int run( QgsFeedback *feedback );

  private:

    const QString mProcess;
    const QStringList mArguments;
    std::function< void( const QByteArray & ) > mStdoutHandler;
    std::function< void( const QByteArray & ) > mStderrHandler;

};


///@cond PRIVATE
#ifndef SIP_RUN

class ProcessThread : public QThread
{
    Q_OBJECT

  public:
    ProcessThread( const std::function<void()> &function, QObject *parent = nullptr )
      : QThread( parent )
      , mFunction( function )
    {
    }

    void run() override
    {
      mFunction();
    }

  private:
    std::function<void()> mFunction;
};

#endif
///@endcond


#endif
