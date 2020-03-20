#include "jobscheduler.h"

JobScheduler::JobScheduler() = default;

/**
 * Get list of active jobs (JobStatus::Active).
 */
QList<QString> JobScheduler::getActiveJobs()
{
    QList<QString> list;
    list << "foo";
    return list;
}

/**
 * Get list of historic jobs (JobStatus::Done).
 */
QList<QString> JobScheduler::getHistoricJobs()
{
    QList<QString> list;
    list << "foo";
    return list;
}

/**
 * Get details of a specific job.
 */
QList<QString> JobScheduler::getDetailsForJobId(int jobId)
{
    Q_UNUSED(jobId);

    QList<QString> list;
    list << "foo";
    return list;
}
