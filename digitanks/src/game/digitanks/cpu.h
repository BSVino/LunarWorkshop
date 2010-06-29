#ifndef DT_CPU_H
#define DT_CPU_H

#include "structure.h"

class CCPU : public CSupplier
{
	REGISTER_ENTITY_CLASS(CCPU, CSupplier);

public:
								CCPU();

public:
	virtual size_t				InitialDataStrength() { return 3200; };
	virtual size_t				BaseDataFlowPerTurn() { return 50; };

	virtual float				GetBoundingRadius() const { return 8; };

	virtual void				SetupMenu(menumode_t eMenuMode);

	bool						IsPreviewBuildValid() const;

	Vector						GetPreviewBuild() const { return m_vecPreviewBuild; };
	virtual void				SetPreviewBuild(Vector vecPreviewBuild);
	void						ClearPreviewBuild();

	void						BeginConstruction();
	void						CancelConstruction();

	virtual void				StartTurn();
	virtual void				PostStartTurn();

	virtual void				OnRender();
	virtual void				PostRender();

protected:
	Vector						m_vecPreviewBuild;

	CEntityHandle<CStructure>	m_hConstructing;
};

#endif