#include "LVRPointInfo.hpp"

namespace lvr
{

LVRPointInfo::LVRPointInfo(QWidget* parent, PointBufferPtr pointBuffer, int pointId) :
    QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_pointInfo.setupUi(this);

    size_t n;
    floatArr points = pointBuffer->getPointArray(n);

    if (pointId < 0 || pointId >= n)
    {
        done(0);
        return;
    }

    m_pointInfo.infoText->setText(QString("Selected: %1, %2, %3")
        .arg(points[pointId * 3], 10, 'g', 4)
        .arg(points[pointId * 3 + 1], 10, 'g', 4)
        .arg(points[pointId * 3 + 2], 10, 'g', 4));
    
    size_t n_spec;
    floatArr spec = pointBuffer->getPointSpectralChannelsArray(n_spec, m_numChannels);
    
    if (pointId >= n_spec)
    {
        done(0);
        return;
    }
    else
    {
        m_data = floatArr(new float[m_numChannels]);
        for (int i = 0; i < m_numChannels; i++)
        {
            m_data[i] = spec[pointId * m_numChannels + i];
        }
    }

    refresh();

    QObject::connect(m_pointInfo.shouldScale, SIGNAL(stateChanged(int)), this, SLOT(refresh()));
}

LVRPointInfo::~LVRPointInfo()
{
    // TODO Auto-generated destructor stub
}

void LVRPointInfo::refresh()
{
    if (m_pointInfo.shouldScale->isChecked())
    {
        m_pointInfo.plotter->setPoints(m_data, m_numChannels);
    }
    else
    {
        m_pointInfo.plotter->setPoints(m_data, m_numChannels, 0, 1);
    }

    show();
    raise();
    activateWindow();
}

}