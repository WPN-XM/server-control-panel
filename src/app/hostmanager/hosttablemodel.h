#ifndef HOSTTABLEMODEL_H
#define HOSTTABLEMODEL_H

#include <utility>

#include <QAbstractTableModel>
#include <QList>

#include "host.h"

namespace HostsFileManager
{
    class HostsTableModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit HostsTableModel(QObject *parent = nullptr);

        enum Columns
        {
            COLUMN_ADDRESS = 0,
            COLUMN_NAME    = 1,
        };

        int rowCount(const QModelIndex &parent) const override;
        int columnCount(const QModelIndex &parent) const override;
        QVariant data(const QModelIndex &index, int role) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
        bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

        void setList(QList<Host *> hosts);
        QList<Host *> getList();

    signals:

    public slots:

    private:
        QList<Host *> listHosts;
    };
} // namespace HostsFileManager

#endif // HOSTTABLEMODEL_H
