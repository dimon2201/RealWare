#include "TRF.hpp"

using namespace triton::resource_file;

int main(int argc, char** argv)
{
	CResourceFile<EResourceFormat::Model3D> m3d(
		"C:/My/My_Projects_Theoretical/Game_TerkonProtocol/Building5_Dif.fbx"
	);
	m3d.Parse();

	system("pause");

	return 0;
}