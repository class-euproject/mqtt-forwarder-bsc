#include "LogWriter.h"

namespace fog {

/******************************************************************************
 * Checks to see if a directory exists. Note: This method only checks the
 * existence of the full path AND if path leaf is a dir.
 *
 * @return  >0 if dir exists AND is a dir,
 *           0 if dir does not exist OR exists but not a dir,
 *          <0 if an error occurred (errno is also set)
 *****************************************************************************/
int dirExists(const char* const path)
{
    struct stat info;

    int statRC = stat( path, &info );
    if( statRC != 0 )
    {
        if (errno == ENOENT)  { return 0; } // something along the path does not exist
        if (errno == ENOTDIR) { return 0; } // something in path prefix is not a dir
        return -1;
    }

    return ( info.st_mode & S_IFDIR ) ? 1 : 0;
}

LogWriter::LogWriter(std::string fileSavingPath) {
    path = fileSavingPath;
    if(dirExists(path.c_str()) <= 0){
        system( ("mkdir -p " + path).c_str() );
    }

}

std::string LogWriter::createSubDirectories() {
    std::string new_folder = path + std::to_string(tmRealYear);
    if(!mkdir((const char *)new_folder.c_str(), 0777)){
        ;//std::cout<<"create year folder\n";
    }
    new_folder = new_folder + separator() + std::to_string(tmYday);
    if(!mkdir((const char *)new_folder.c_str(), 0777)){
        ;//std::cout<<"create day folder\n";
    }
    new_folder = new_folder + separator() + std::to_string(tmHour);
    if(!mkdir((const char *)new_folder.c_str(), 0777)){
        ;//std::cout<<"create hour folder\n";
    }
    return new_folder + separator();
}

std::string LogWriter::getTimePath() {
    time(&rawtime);
    tmStruct = localtime(&rawtime);
    tmYear = tmStruct->tm_year;
    tmRealYear = tmYear + 1900;
    tmHour = tmStruct->tm_hour;
    tmYday = tmStruct->tm_yday;
    tmMin = tmStruct->tm_min;
    return createSubDirectories() + std::to_string(tmMin);
}

void LogWriter::write(MasaMessage m) {
    std::string filename = getTimePath() + "_" + std::to_string(m.cam_idx) + ".txt";
    std::ofstream of;
    of.open(filename, std::ios_base::app);
    for(auto const& ru: m.objects) {
        of << m.cam_idx << " " << m.t_stamp_ms << " " << ru.category << " "
           << std::fixed << std::setprecision(9) << ru.latitude << " " << ru.longitude << " "
           << std::fixed << std::setprecision(2) << static_cast<float>(ru.speed) << " " << static_cast<float>(ru.orientation) << " "
           << static_cast<double>(ru.error) << " " << ru.camera_id.size() << " ";
        for(size_t i=0; i<ru.camera_id.size(); i++)
            of << ru.camera_id.at(i) << " ";
        for(size_t i=0; i<ru.object_id.size(); i++)
            of << ru.object_id.at(i) << " ";
        of << "\n";
    }
    for(auto const& l: m.lights) {
        of << m.cam_idx << " " << m.t_stamp_ms << " " << l.status << " "
           << std::fixed << std::setprecision(9) << l.latitude << " " << l.longitude << " "
           << std::fixed << std::setprecision(2) << static_cast<float>(l.time_to_change) << " " << static_cast<float>(l.orientation) << "\n";
    }
    of.close();
}
}