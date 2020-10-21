#pragma once

#include <FS.h>


class FileScanner {
  public:
    FileScanner(fs::FS &f) : fs(f) {}
    void    menu(const char* title);

  protected:
    bool    is_text_by_extension(const char* fname);
    bool    is_image_by_extension(const char* fname);
    void    edit_file(const char* fname);
    void    display_file(const char* fname);
    void    file_info(const char* fname);
    bool    delete_file(const char* fname);
    void    hex_dump(const char* fname);
    bool    process_file(const char* fname);
    void    create_menu(ezMenu& m, File dir);
    fs::FS& fs;
    const char* text_extensions[7]  = { ".txt", ".log", ".json", ".xml", ".html", ".css", ".py" };
    const char* image_extensions[3] = { ".bmp", ".jpg", ".png" };
};
