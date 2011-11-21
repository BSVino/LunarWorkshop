#include <stdio.h>
#include <memory>

#include <toys/toy_util.h>
#include <toys/toy.h>

void test_toys()
{
	CToyUtil t;

	t.AddMaterial("test.png");
	t.AddMaterial("test2.png");
	t.AddMaterial("test0.png");
	t.AddMaterial("test3.png");
	t.AddVertex(0, Vector(1, 2, 3), Vector2D(0, 0));
	t.AddVertex(0, Vector(1, 3, 3), Vector2D(0, 1));
	t.AddVertex(0, Vector(1, 2, 4), Vector2D(1, 1));
	t.AddVertex(0, Vector(2, 2, 3), Vector2D(1, 0));
	t.AddVertex(0, Vector(2, 3, 3), Vector2D(1, 1));
	t.AddVertex(0, Vector(2, 2, 4), Vector2D(1, 0));
	t.AddVertex(1, Vector(10, 20, 30), Vector2D(0, 0));
	t.AddVertex(1, Vector(10, 30, 30), Vector2D(0, 10));
	t.AddVertex(1, Vector(10, 20, 40), Vector2D(10, 10));
	t.AddVertex(1, Vector(20, 20, 30), Vector2D(10, 0));
	t.AddVertex(1, Vector(20, 30, 30), Vector2D(10, 10));
	t.AddVertex(1, Vector(20, 20, 40), Vector2D(10, 0));
	t.AddVertex(1, Vector(11, 12, 13), Vector2D(10, 10));
	t.AddVertex(1, Vector(11, 13, 13), Vector2D(10, 11));
	t.AddVertex(1, Vector(11, 12, 14), Vector2D(11, 11));
	// Skip the third, it has no verts and should get op'd out.
	t.AddVertex(3, Vector(12, 12, 13), Vector2D(11, 10));
	t.AddVertex(3, Vector(12, 13, 13), Vector2D(11, 11));
	t.AddVertex(3, Vector(12, 12, 14), Vector2D(11, 10));
	t.AddPhysVertex(Vector(0, 0, 1));
	t.AddPhysVertex(Vector(0, 1, 1));
	t.AddPhysVertex(Vector(0, 1, 0));
	t.AddPhysVertex(Vector(0, 0, 0));
	t.AddPhysTriangle(0, 1, 2);
	t.AddPhysTriangle(0, 2, 3);
	TAssert(t.Write("test.toy"));

	CToy* pToy = new CToy();

	t.Read("test.toy", pToy);
	TAssert(strcmp(pToy->GetMaterialTexture(0), "test.png") == 0);
	TAssert(strcmp(pToy->GetMaterialTexture(1), "test2.png") == 0);
	TAssert(strcmp(pToy->GetMaterialTexture(2), "test3.png") == 0);
	TAssert(Vector(pToy->GetMaterialVert(0, 0)) == Vector(1, 2, 3));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 0)+3) == Vector2D(0, 0));
	TAssert(Vector(pToy->GetMaterialVert(0, 1)) == Vector(1, 3, 3));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 1)+3) == Vector2D(0, 1));
	TAssert(Vector(pToy->GetMaterialVert(0, 2)) == Vector(1, 2, 4));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 2)+3) == Vector2D(1, 1));
	TAssert(Vector(pToy->GetMaterialVert(0, 3)) == Vector(2, 2, 3));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 3)+3) == Vector2D(1, 0));
	TAssert(Vector(pToy->GetMaterialVert(0, 4)) == Vector(2, 3, 3));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 4)+3) == Vector2D(1, 1));
	TAssert(Vector(pToy->GetMaterialVert(0, 5)) == Vector(2, 2, 4));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 5)+3) == Vector2D(1, 0));
	TAssert(Vector(pToy->GetMaterialVert(1, 0)) == Vector(10, 20, 30));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 0)+3) == Vector2D(0, 0));
	TAssert(Vector(pToy->GetMaterialVert(1, 1)) == Vector(10, 30, 30));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 1)+3) == Vector2D(0, 10));
	TAssert(Vector(pToy->GetMaterialVert(1, 2)) == Vector(10, 20, 40));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 2)+3) == Vector2D(10, 10));
	TAssert(Vector(pToy->GetMaterialVert(1, 3)) == Vector(20, 20, 30));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 3)+3) == Vector2D(10, 0));
	TAssert(Vector(pToy->GetMaterialVert(1, 4)) == Vector(20, 30, 30));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 4)+3) == Vector2D(10, 10));
	TAssert(Vector(pToy->GetMaterialVert(1, 5)) == Vector(20, 20, 40));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 5)+3) == Vector2D(10, 0));
	TAssert(Vector(pToy->GetMaterialVert(1, 6)) == Vector(11, 12, 13));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 6)+3) == Vector2D(10, 10));
	TAssert(Vector(pToy->GetMaterialVert(1, 7)) == Vector(11, 13, 13));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 7)+3) == Vector2D(10, 11));
	TAssert(Vector(pToy->GetMaterialVert(1, 8)) == Vector(11, 12, 14));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 8)+3) == Vector2D(11, 11));
	TAssert(Vector(pToy->GetMaterialVert(2, 0)) == Vector(12, 12, 13));
	TAssert(Vector2D(pToy->GetMaterialVert(2, 0)+3) == Vector2D(11, 10));
	TAssert(Vector(pToy->GetMaterialVert(2, 1)) == Vector(12, 13, 13));
	TAssert(Vector2D(pToy->GetMaterialVert(2, 1)+3) == Vector2D(11, 11));
	TAssert(Vector(pToy->GetMaterialVert(2, 2)) == Vector(12, 12, 14));
	TAssert(Vector2D(pToy->GetMaterialVert(2, 2)+3) == Vector2D(11, 10));

	pToy->DeallocateMesh();

	TAssert(pToy->GetMaterialNumVerts(0) == 6);
	TAssert(pToy->GetMaterialNumVerts(1) == 9);
	TAssert(pToy->GetMaterialNumVerts(2) == 3);
	TAssert(pToy->GetAABB().m_vecMins == Vector(1, 2, 3));
	TAssert(pToy->GetAABB().m_vecMaxs == Vector(20, 30, 40));
	TAssert(pToy->GetNumMaterials() == 3);

	TAssert(pToy->GetPhysicsNumTris() == 2);
	TAssert(pToy->GetPhysicsNumVerts() == 4);
	TAssert(pToy->GetPhysicsTri(0)[0] == 0);
	TAssert(pToy->GetPhysicsTri(0)[1] == 1);
	TAssert(pToy->GetPhysicsTri(0)[2] == 2);
	TAssert(pToy->GetPhysicsTri(1)[0] == 0);
	TAssert(pToy->GetPhysicsTri(1)[1] == 2);
	TAssert(pToy->GetPhysicsTri(1)[2] == 3);
	TAssert(Vector(pToy->GetPhysicsVert(0)) == Vector(0, 0, 1));
	TAssert(Vector(pToy->GetPhysicsVert(1)) == Vector(0, 1, 1));
	TAssert(Vector(pToy->GetPhysicsVert(2)) == Vector(0, 1, 0));
	TAssert(Vector(pToy->GetPhysicsVert(3)) == Vector(0, 0, 0));

	delete pToy;
}
