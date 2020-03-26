#ifndef MOUNTEDDISKINFO_H
#define MOUNTEDDISKINFO_H

#include <QObject>

enum class VolumeSizeUnit
{
    KB, MB, GB
};

class MountedDiskInfo
{
    Q_GADGET

    Q_PROPERTY(QString letter READ letter)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString volume READ volume)

public:
    MountedDiskInfo() = default;
    MountedDiskInfo(const QString& url, const QString& letter, int volume, VolumeSizeUnit unit);

    const QString& letter() const;
    const QString& path() const;
    const QString& volume() const;

    void setLetter(const QString& letter);
    void setPath(const QString& path);
    void setVolume(const QString& volume);

private:
    QString m_letter;
    QString m_path;
    QString m_volume;
};

#endif // MOUNTEDDISKINFO_H
