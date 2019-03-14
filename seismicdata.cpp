#include "seismicdata.h"

seismicData::seismicData()
{
    m_dataType[GRISYS] = "GRISYS";
    m_dataType[SEG2] = "SEG2";
    m_dataType[SEGD] = "SEGD";
    m_dataType[SEGY] = "SEGY";
    m_dataType[GEOEAST] = "GEOEAST";
    m_dataType[NOT_SUPPORT] = "NOT_SUPPORT";
}
