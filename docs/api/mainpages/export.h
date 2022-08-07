/**

@page document-export Saving and exporting documents


Lomse export documents as strings in MusicXML format or LDP format. But currently Lomse does not include facilities for writing files (although this is planned for future). Therefore, for now it is responsibility of your application to save in a file the source code string provided by Lomse.

Exporting a document is a simple operation. It is just instantiating the exporter object for the required format (MxlExporter for MusicXML format or LdpExporter for LDP format), specify the desired generation options and request to generate the source code. Example:

@code
    ofstream file1(path + "musicxml_export.xml", ios::out);
    if (file1.good())
    {
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_separator_lines(true);
        ImoScore* pScore = ...
        std::string source = exporter.get_source(pScore);
        file1.write(source.c_str(), source.size());
        file1.close();
    }
    else
    {
        std::cout << "file error write" << endl;
        CHECK( false );
    }
@endcode

Plain text MusicXML files can be very large and, therefore, MusicXML files are normally exported in compressed format, which reduces file sizes to roughly the same size as the corresponding MIDI files. The format uses zip compression and the ".mxl" file extension. If your application would like to export in compressed format, this will have to be done in your application. See 
https://www.w3.org/2021/06/musicxml40/tutorial/compressed-mxl-files/

For now, Lomse does not include facilities for writing files and generating the MusicXML compressed format, but this is planned for future.

Exporting in LDP format is done in the same way, but using the LdpExporter object. Example:

@code
    ofstream file1(path + "ldp_export.lms", ios::out);
    if (file1.good())
    {
        LdpExporter exporter;
        ImoScore* pScore = ...
        std::string source = exporter.get_source(pScore);
        file1.write(source.c_str(), source.size());
        file1.close();
    }
    else
    {
        std::cout << "file error write" << endl;
        CHECK( false );
    }
@endcode

*/

