#include "tasset.hpp"

using namespace triton::asset;

int main(int argc, char** argv)
{
	CModel3DAsset resource;
	resource.LoadRawFile("C:/My/My_Projects_Theoretical/Game_TerkonProtocol/Building5_Dif.fbx");
	resource.WriteAssetFile("C:/My/My_Projects_Theoretical/Game_TerkonProtocol/Building5_Dif.trf");
	resource.LoadAssetFile("C:/My/My_Projects_Theoretical/Game_TerkonProtocol/Building5_Dif.trf");

	system("pause");

	return 0;
}