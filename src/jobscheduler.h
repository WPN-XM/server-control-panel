#ifndef JOBSCHEDULER_H
#define JOBSCHEDULER_H

#include <QObject>

class JobScheduler
{
public:
    JobScheduler();

    QList<QString> getActiveJobs();
    QList<QString> getHistoricJobs();
    QList<QString> getDetailsForJobId(int jobId);
};

#endif // JOBSCHEDULER_H
