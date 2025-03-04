static int ric_file_open_write(struct ric_file *_file, const char *filename,
                                u32 version)
{
    enum ric_error err = RIC_SUCCESS;

    // Open file for write
    _file->fs = fopen(filename, "wb");
    if (!_file->fs)
    {
        ric_file_close(_file);
        fprintf(stderr, "Unable to open %s for writing\n", filename);
        return RICO_ERROR(RIC_ERR_FILE_WRITE, "Failed to open file %s for write",
                          filename);
    }

    // File version
    if (version < RICO_FILE_VERSION_MINIMUM_SUPPORTED)
    {
        ric_file_close(_file);
        fprintf(stderr, "Unsupported file version [%d]. The minimum supported" \
                "version for this build is [%d].\n", version,
                RICO_FILE_VERSION_MAXIMUM_SUPPORTED);
        return RICO_ERROR(RIC_ERR_FILE_VERSION,
                          "Unsupported file version %d while writing file %s",
                          version, filename);
    }
    else if (version > RICO_FILE_VERSION_MAXIMUM_SUPPORTED)
    {
        ric_file_close(_file);
        fprintf(stderr, "Unsupported file version [%d]. The maximum supported" \
                "version for this build is [%d].\n", version,
                RICO_FILE_VERSION_MAXIMUM_SUPPORTED);
        return RICO_ERROR(RIC_ERR_FILE_VERSION,
                          "Unsupported file version %d while writing file %s",
                          version, filename);
    }

    _file->version = version;
    _file->filename = filename;
    _file->next_uid = 0;

    // TODO: Separate dynamic data from static data. Textures, meshes, and
    //       static objects should be in their own file. Dynamic objects
    //       should be loaded as part of the save file. Save file can also
    //       override static objects' positions, states, etc.

    // File signature and version
    fwrite(&PACK_SIGNATURE,  sizeof(PACK_SIGNATURE),  1, _file->fs);
    fwrite(&_file->version,  sizeof(_file->version),  1, _file->fs);
    fwrite(&_file->next_uid, sizeof(_file->next_uid), 1, _file->fs);

    return err;
}
static int ric_file_open_read(struct ric_file *_file, const char *filename)
{
    enum ric_error err = RIC_SUCCESS;

    // Open file for read
    _file->fs = fopen(filename, "rb");
    if (!_file->fs)
    {
        ric_file_close(_file);
        fprintf(stderr, "Unable to open %s for reading\n", filename);
        return RICO_ERROR(RIC_ERR_FILE_READ, "Failed to open %s for read",
                          filename);
    }

    // TODO: Wrap fread() calls in something that checks if fread() return
    //       value (bytes read) is less than requested bytes. If so, check
    //       for EOF with feof() or error with ferror().

    // File signature
    u8 signature[4] = { 0 };
    fread(&signature, sizeof(signature), 1, _file->fs);
    if (memcmp(signature, PACK_SIGNATURE, sizeof(signature)))
    {
        ric_file_close(_file);
        fprintf(stderr, "Invalid file signature: %s\n", signature);
        return RICO_ERROR(RIC_ERR_FILE_SIGNATURE,
                          "Invalid file signature %d in file %s",
                          signature, filename);
    }

    // File version
    fread(&_file->version, sizeof(_file->version), 1, _file->fs);
    if (_file->version < RICO_FILE_VERSION_MINIMUM_SUPPORTED)
    {
        ric_file_close(_file);
        fprintf(stderr, "Unsupported file version [%d]. The minimum supported" \
                "version for this build is [%d].\n", _file->version,
                RICO_FILE_VERSION_MAXIMUM_SUPPORTED);
        return RICO_ERROR(RIC_ERR_FILE_VERSION,
                          "Unsupported file version %d in file %s",
                          _file->version, filename);
    }
    else if (_file->version > RICO_FILE_VERSION_MAXIMUM_SUPPORTED)
    {
        ric_file_close(_file);
        fprintf(stderr, "Unsupported file version [%d]. The maximum supported" \
                "version for this build is [%d].\n", _file->version,
                RICO_FILE_VERSION_MAXIMUM_SUPPORTED);
        return RICO_ERROR(RIC_ERR_FILE_VERSION,
                          "Unsupported file version %d in file %s",
                          _file->version, filename);
    }

    // Next available uid
    fread(&_file->next_uid, sizeof(_file->next_uid), 1, _file->fs);

    _file->filename = filename;
    return err;
}
static void ric_file_close(struct ric_file *handle)
{
    if (handle->fs) {
        fclose(handle->fs);
        handle->fs = NULL;
    }
}
