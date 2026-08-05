#pragma once

class LanMode
{
public:
	LanMode(bool isLanMode)
	{
		static bool bInitialised = false;
		if (bInitialised)
			return;

		bInitialised = true;
		m_bIsLanMode = isLanMode;
	}

	bool Enabled() { return m_bIsLanMode; }

private:
	bool m_bIsLanMode = false;
};

inline LanMode* g_LanMode;
