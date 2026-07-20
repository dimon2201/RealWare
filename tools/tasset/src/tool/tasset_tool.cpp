#include "tasset.hpp"

using namespace triton::asset;

void PrintHelp()
{
	Print("tasset - Triton Asset Tool");
	Print("");
	Print("Usage:");
	Print("  tasset <command> <input>");
	Print("");
	Print("Commands:");
	Print("  -c     Convert");
	Print("  -h     Show help message");
	Print("");
	Print("Examples:");
	Print("  tasset -c knight.fbx knight.tasset");
}

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		std::string flag = std::string(argv[1]);

		if (flag == "-h")
		{
			PrintHelp();
			return 0;
		}
		else if (flag == "-c")
		{
			if (argc == 3)
			{
				std::filesystem::path inFilePath = std::filesystem::path(argv[2]);
				if (inFilePath.extension() == ".fbx")
				{
					Print("Info: converting FBX 3d model file...");

					const std::string inFilePathStr = inFilePath.generic_string();
					const std::string outFilePathStr = std::string(inFilePath.replace_extension(".tasset").generic_string());

					CModel3DAsset asset;
					asset.LoadRawFile(inFilePathStr);
					asset.WriteAssetFile(outFilePathStr);

					Print("Info: file was converted successfully!");

					return 0;
				}
			}
			else
			{
				Print("Error: insufficient argument count");
			}
		}
	}

	PrintHelp();

	return 0;
}