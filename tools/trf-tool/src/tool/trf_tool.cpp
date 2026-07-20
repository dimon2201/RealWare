#include "TRF.hpp"

using namespace triton::resource_file;

int main(int argc, char** argv)
{
	CModel3DResource resource;
	resource.LoadRawFile("C:/My/My_Projects_Theoretical/Game_TerkonProtocol/Building5_Dif.fbx");
	resource.WriteResourceFile("C:/My/My_Projects_Theoretical/Game_TerkonProtocol/Building5_Dif.trf");
	resource.LoadResourceFile("C:/My/My_Projects_Theoretical/Game_TerkonProtocol/Building5_Dif.trf");

	system("pause");

	return 0;
}