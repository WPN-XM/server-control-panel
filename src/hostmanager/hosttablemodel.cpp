#include "hosttablemodel.h"

namespace HostsFileManager
{
    HostsTableModel::HostsTableModel(QObject *parent) : QAbstractTableModel(parent) {}

    int HostsTableModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return listHosts.size();
    }

    int HostsTableModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 2;
    }

    QVariant HostsTableModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= listHosts.size() || index.row() < 0)
            return QVariant();

        Host *host = listHosts.at(index.row());

        /*
  if (role == Qt::CheckStateRole){
      switch(index.column()){
      case COLUMN_ADDRESS:
          return host->isEnable()?Qt::Checked:Qt::Unchecked;
      }
  }*/
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case COLUMN_ADDRESS:
                return host->ipaddress;
            case COLUMN_NAME:
                return host->hostname;
            }
        }
        return QVariant();
    }

    QVariant HostsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal) {
            switch (section) {
            case COLUMN_ADDRESS:
                return "Address";
            case COLUMN_NAME:
                return "Name";
            default:
                return QVariant();
            }
        }

        return QVariant();
    }

    Qt::ItemFlags HostsTableModel::flags(const QModelIndex &index) const
    {
        if (!index.isValid())
            return Qt::ItemIsEnabled;

        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }

    bool HostsTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (index.isValid() && role == Qt::EditRole) {
            int row = index.row();

            Host *host = listHosts.value(row);

            switch (index.column()) {
            case COLUMN_ADDRESS:
                host->ipaddress = value.toString();
                break;
            case COLUMN_NAME:
                host->hostname = value.toString();
                break;
            default:
                return false;
            }

            emit(dataChanged(index, index));

            return true;
        }

        return false;
    }

    bool HostsTableModel::insertRows(int position, int rows, const QModelIndex &index)
    {
        Q_UNUSED(index);
        beginInsertRows(QModelIndex(), position, position + rows - 1);

        for (int row = 0; row < rows; row++) {
            auto *host = new Host();
            listHosts.insert(position, host);
        }

        endInsertRows();
        return true;
    }

    bool HostsTableModel::removeRows(int position, int rows, const QModelIndex &index)
    {
        Q_UNUSED(index);
        beginRemoveRows(QModelIndex(), position, position + rows - 1);

        for (int row = 0; row < rows; ++row) {
            Host *host = listHosts.takeAt(position);
            delete host;
        }

        endRemoveRows();
        return true;
    }

    void HostsTableModel::setList(QList<Host *> hosts)
    {
        listHosts        = hosts;
        QModelIndex root = index(0, 0);
        emit(dataChanged(root, index(rowCount(QModelIndex()), columnCount(QModelIndex()))));
    }

    QList<Host *> HostsTableModel::getList() { return listHosts; }
} // namespace HostsFileManager
