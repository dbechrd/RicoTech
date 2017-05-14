//#include "rico_file.h"
//#include <string.h>

#define SIGNATURE_SIZE 4
static const char SIGNATURE[SIGNATURE_SIZE] = { 'R', '1', 'C', '0' };

int rico_file_open_write(struct rico_file *_file, const char *filename,
                         u32 version)
{
    enum rico_error err = SUCCESS;

    // Open file for write
    _file->fs = fopen(filename, "wb");
    if (!_file->fs)
    {
        rico_file_close(_file);
        fprintf(stderr, "Unable to open %s for writing\n", filename);
        return RICO_ERROR(ERR_FILE_WRITE, "Failed to open file %s for write",
                          filename);
    }

    // File version
    if (version < RICO_FILE_VERSION_MINIMUM_SUPPORTED)
    {
        rico_file_close(_file);
        fprintf(stderr, "Unsupported file version [%d]. The minimum supported" \
                "version for this build is [%d].\n", version,
                RICO_FILE_VERSION_MAXIMUM_SUPPORTED);
        return RICO_ERROR(ERR_FILE_VERSION,
                          "Unsupported file version %d while writing file %s",
                          version, filename);
    }
    else if (version > RICO_FILE_VERSION_MAXIMUM_SUPPORTED)
    {
        rico_file_close(_file);
        fprintf(stderr, "Unsupported file version [%d]. The maximum supported" \
                "version for this build is [%d].\n", version,
                RICO_FILE_VERSION_MAXIMUM_SUPPORTED);
        return RICO_ERROR(ERR_FILE_VERSION,
                          "Unsupported file version %d while writing file %s",
                          version, filename);
    }
    
    _file->version = version;
    _file->filename = filename;
    _file->cereal_index = version - RICO_FILE_VERSION_MINIMUM_SUPPORTED;

    // TODO: Separate dynamic data from static data. Textures, meshes, and
    //       static objects should be in their own file. Dynamic objects
    //       should be loaded as part of the save file. Save file can also
    //       override static objects' positions, states, etc.

    // File signature and version
    fwrite(SIGNATURE,       SIGNATURE_SIZE,         1, _file->fs);
    fwrite(&_file->version, sizeof(_file->version), 1, _file->fs);

    return err;
}

int rico_file_open_read(struct rico_file *_file, const char *filename)
{
    enum rico_error err = SUCCESS;

    // Open file for read
    _file->fs = fopen(filename, "rb");
    if (!_file->fs)
    {
        rico_file_close(_file);
        fprintf(stderr, "Unable to open %s for reading\n", filename);
        return RICO_ERROR(ERR_FILE_READ, "Failed to open %s for read",
                          filename);
    }

    // TODO: Wrap fread() calls in something that checks if fread() return
    //       value (bytes read) is less than requested bytes. If so, check
    //       for EOF with feof() or error with ferror().

    // File signature
    char SIGNATURE_BUFFER[SIGNATURE_SIZE];
    fread(&SIGNATURE_BUFFER, SIGNATURE_SIZE, 1, _file->fs);
    if (strncmp(SIGNATURE_BUFFER, SIGNATURE, SIGNATURE_SIZE))
    {
        rico_file_close(_file);
        fprintf(stderr, "Invalid file signature: %s\n", SIGNATURE_BUFFER);
        return RICO_ERROR(ERR_FILE_SIGNATURE,
                          "Invalid file signature %s in file %s",
                          SIGNATURE_BUFFER, filename);
    }

    // File version
    fread(&_file->version, sizeof(_file->version), 1, _file->fs);
    if (_file->version < RICO_FILE_VERSION_MINIMUM_SUPPORTED)
    {
        rico_file_close(_file);
        fprintf(stderr, "Unsupported file version [%d]. The minimum supported" \
                "version for this build is [%d].\n", _file->version,
                RICO_FILE_VERSION_MAXIMUM_SUPPORTED);
        return RICO_ERROR(ERR_FILE_VERSION,
                          "Unsupported file version %d in file %s",
                          _file->version, filename);
    }
    else if (_file->version > RICO_FILE_VERSION_MAXIMUM_SUPPORTED)
    {
        rico_file_close(_file);
        fprintf(stderr, "Unsupported file version [%d]. The maximum supported" \
                "version for this build is [%d].\n", _file->version,
                RICO_FILE_VERSION_MAXIMUM_SUPPORTED);
        return RICO_ERROR(ERR_FILE_VERSION,
                          "Unsupported file version %d in file %s",
                          _file->version, filename);
    }

    _file->cereal_index = _file->version - RICO_FILE_VERSION_MINIMUM_SUPPORTED;
    _file->filename = filename;
    return err;
}

void rico_file_close(struct rico_file *handle)
{
    if (handle->fs) {
        fclose(handle->fs);
        handle->fs = NULL;
    }
}