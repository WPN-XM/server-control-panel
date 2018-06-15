#ifndef HOSTTABLEMODEL_H
#define HOSTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>

#include "host.h"

namespace HostsFileManager
{
    class HostsTableModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit HostsTableModel(QObject *parent = 0);

        enum Columns
        {
            COLUMN_ADDRESS = 0,
            COLUMN_NAME    = 1,
        };

        int rowCount(const QModelIndex &parent) const;
        int columnCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
        bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
        bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

        void setList(QList<Host *> listHosts);
        QList<Host *> getList();

    signals:

    public slots:

    private:
        QList<Host *> listHosts;
    };
}

#endif // HOSTTABLEMODEL_H
