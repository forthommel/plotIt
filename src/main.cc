// For fnmatch()
#include <fnmatch.h>

#include <TROOT.h>
#include <TList.h>
#include <TCollection.h>
#include <TCanvas.h>
#include <TError.h>
#include <TFile.h>
#include <TKey.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <TPaveText.h>
#include <TColor.h>
#include <TGaxis.h>
#include <Math/QuantFuncMathCore.h>

#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <set>
#include <iomanip>

#include "tclap/CmdLine.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <commandlinecfg.h>
#include <plotters.h>
#include <pool.h>
#include <summary.h>
#include <systematics.h>
#include <utilities.h>
#include <plotIt.h>

namespace fs = boost::filesystem;
using std::setw;

// Load libdpm at startup, on order to be sure that rfio files are working
#include <dlfcn.h>
struct Dummy
{
  Dummy()
  {
    dlopen("libdpm.so", RTLD_NOW|RTLD_GLOBAL);
  }
};
static Dummy foo;


int main(int argc, char** argv) {

  try {

    TCLAP::CmdLine cmd("Plot histograms", ' ', "0.1");

    TCLAP::ValueArg<std::string> histogramsFolderArg("i", "histograms-folder", "histograms base folder (default: current directory)", false, "./", "string", cmd);

    TCLAP::ValueArg<std::string> outputFolderArg("o", "output-folder", "output folder", true, "", "string", cmd);

    TCLAP::ValueArg<std::string> eraArg("e", "era", "era to restrict to", false, "", "string", cmd);

    TCLAP::SwitchArg ignoreScaleArg("", "ignore-scales", "Ignore any scales present in the configuration file", cmd, false);

    TCLAP::SwitchArg verboseArg("v", "verbose", "Verbose output (print summary)", cmd, false);

    TCLAP::SwitchArg yieldsArg("y", "yields", "Produce LaTeX table of yields", cmd, false);

    TCLAP::SwitchArg plotsArg("p", "plots", "Do not produce the plots - can be useful if only the yields table is needed", cmd, false);

    TCLAP::SwitchArg unblindArg("u", "unblind", "Unblind the plots, ie ignore any blinded-range in the configuration", cmd, false);

    TCLAP::SwitchArg systematicsBreakdownArg("b", "systs-breadown", "Print systematics details for each MC process separately in addition to the total contribution", cmd, false);

    TCLAP::UnlabeledValueArg<std::string> configFileArg("configFile", "configuration file", true, "", "string", cmd);

    cmd.parse(argc, argv);

    //bool isData = dataArg.isSet();

    fs::path histogramsPath(fs::canonical(histogramsFolderArg.getValue()));

    if (! fs::exists(histogramsPath)) {
      std::cout << "Error: histograms path " << histogramsPath << " does not exist" << std::endl;
    }

    fs::path outputPath(outputFolderArg.getValue());

    if (! fs::exists(outputPath)) {
      std::cout << "Error: output path " << outputPath << " does not exist" << std::endl;
      return 1;
    }

    if( plotsArg.getValue() && !yieldsArg.getValue() ) {
      std::cerr << "Error: we have nothing to do" << std::endl;
      return 1;
    }

    CommandLineCfg::get().era = eraArg.getValue();
    CommandLineCfg::get().ignore_scales = ignoreScaleArg.getValue();
    CommandLineCfg::get().verbose = verboseArg.getValue();
    CommandLineCfg::get().do_plots = !plotsArg.getValue();
    CommandLineCfg::get().do_yields = yieldsArg.getValue();
    CommandLineCfg::get().unblind = unblindArg.getValue();
    CommandLineCfg::get().systematicsBreakdown = systematicsBreakdownArg.getValue();

    plotIt::plotIt p(outputPath);
    if (!p.parseConfigurationFile(configFileArg.getValue(), histogramsPath))
        return 1;

    p.plotAll();

  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }

  return 0;
}
