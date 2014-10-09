/**************************************************************************
**
** Copyright (C) 2012-2014 Digia Plc and/or its subsidiary(-ies).
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

#include "binaryformatengine.h"

namespace {

class StringListIterator : public QAbstractFileEngineIterator
{
public:
    StringListIterator( const QStringList &list, QDir::Filters filters, const QStringList &nameFilters)
        : QAbstractFileEngineIterator(filters, nameFilters)
        , list(list)
        , index(-1)
    {
    }

    bool hasNext() const
    {
        return index < list.size() - 1;
    }

    QString next()
    {
        if(!hasNext())
            return QString();
        ++index;
        return currentFilePath();
    }

    QString currentFileName() const
    {
        return index < 0 ? QString() : list[index];
    }

private:
    const QStringList list;
    int index;
};

} // anon namespace

namespace QInstaller {

BinaryFormatEngine::BinaryFormatEngine(const QHash<QByteArray, ResourceCollection> &collections,
        const QString &fileName)
    : m_resource(0)
    , m_collections(collections)
{
    setFileName(fileName);
}

/*!
    \reimp
*/
void BinaryFormatEngine::setFileName(const QString &file)
{
    m_fileNamePath = file;

    static const QChar sep = QLatin1Char('/');
    static const QString prefix = QLatin1String("installer://");
    Q_ASSERT(file.toLower().startsWith(prefix));

    // cut the prefix
    QString path = file.mid(prefix.length());
    while (path.endsWith(sep))
        path.chop(1);

    m_collection = m_collections.value(path.section(sep, 0, 0).toUtf8());
    m_collection.setName(path.section(sep, 0, 0).toUtf8());
    m_resource = m_collection.resourceByName(path.section(sep, 1, 1).toUtf8());
}

/*!
    \reimp
*/
bool BinaryFormatEngine::close()
{
    if (m_resource.isNull())
        return false;

    const bool result = m_resource->isOpen();
    m_resource->close();
    return result;
}

/*!
    \reimp
*/
bool BinaryFormatEngine::open(QIODevice::OpenMode /*mode*/)
{
    return m_resource.isNull() ? false : m_resource->open();
}

/*!
    \reimp
*/
qint64 BinaryFormatEngine::pos() const
{
    return m_resource.isNull() ? 0 : m_resource->pos();
}

/*!
    \reimp
*/
qint64 BinaryFormatEngine::read(char *data, qint64 maxlen)
{
    return m_resource.isNull() ? -1 : m_resource->read(data, maxlen);
}

/*!
    \reimp
*/
bool BinaryFormatEngine::seek(qint64 offset)
{
    return m_resource.isNull() ? false : m_resource->seek(offset);
}

/*!
    \reimp
*/
QString BinaryFormatEngine::fileName(FileName file) const
{
    static const QChar sep = QLatin1Char('/');
    switch(file) {
        case BaseName:
            return m_fileNamePath.section(sep, -1, -1, QString::SectionSkipEmpty);
        case PathName:
        case AbsolutePathName:
        case CanonicalPathName:
            return m_fileNamePath.section(sep, 0, -2, QString::SectionSkipEmpty);
        case DefaultName:
        case AbsoluteName:
        case CanonicalName:
            return m_fileNamePath;
        default:
            return QString();
    }
}

/*!
    \reimp
*/
bool BinaryFormatEngine::copy(const QString &newName)
{
    if (QFile::exists(newName))
        return false;

    QFile target(newName);
    if (!target.open(QIODevice::WriteOnly))
        return false;

    qint64 bytesLeft = size();
    if (!open(QIODevice::ReadOnly))
        return false;

    char data[4096];
    while(bytesLeft > 0) {
        const qint64 len = qMin<qint64>(bytesLeft, 4096);
        const qint64 bytesRead = read(data, len);
        if (bytesRead != len) {
            close();
            return false;
        }
        const qint64 bytesWritten = target.write(data, len);
        if (bytesWritten != len) {
            close();
            return false;
        }
        bytesLeft -= len;
    }
    close();

    return true;
}

/*!
    \reimp
*/
QAbstractFileEngine::FileFlags BinaryFormatEngine::fileFlags(FileFlags type) const
{
    FileFlags result;
    if ((type & FileType) && (!m_resource.isNull()))
        result |= FileType;
    if ((type & DirectoryType) && m_resource.isNull())
        result |= DirectoryType;
    if ((type & ExistsFlag) && (!m_resource.isNull()))
        result |= ExistsFlag;
    if ((type & ExistsFlag) && m_resource.isNull() && (!m_collection.name().isEmpty()))
        result |= ExistsFlag;

    return result;
}

/*!
    \reimp
*/
QAbstractFileEngineIterator *BinaryFormatEngine::beginEntryList(QDir::Filters filters, const QStringList &filterNames)
{
    const QStringList entries = entryList(filters, filterNames);
    return new StringListIterator(entries, filters, filterNames);
}

/*!
    \reimp
*/
QStringList BinaryFormatEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
    if (!m_resource.isNull())
        return QStringList();

    QStringList result;
    if ((!m_collection.name().isEmpty()) && (filters & QDir::Files)) {
        foreach (const QSharedPointer<Resource> &resource, m_collection.resources())
            result.append(QString::fromUtf8(resource->name()));
    } else if (m_collection.name().isEmpty() && (filters & QDir::Dirs)) {
        foreach (const ResourceCollection &collection, m_collections)
            result.append(QString::fromUtf8(collection.name()));
    }
    result.removeAll(QString()); // Remove empty names, will crash while using directory iterator.

    if (filterNames.isEmpty())
        return result;

    QList<QRegExp> regexps;
    foreach (const QString &i, filterNames)
        regexps.append(QRegExp(i, Qt::CaseInsensitive, QRegExp::Wildcard));

    QStringList entries;
    foreach (const QString &i, result) {
        bool matched = false;
        foreach (const QRegExp &reg, regexps) {
            matched = reg.exactMatch(i);
            if (matched)
                break;
        }
        if (matched)
            entries.append(i);
    }

    return entries;
}

/*!
    \reimp
*/
qint64 BinaryFormatEngine::size() const
{
    return m_resource.isNull() ? 0 : m_resource->size();
}

} // namespace QInstaller
