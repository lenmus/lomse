//---------------------------------------------------------------------------------------
// MusicXmlOptions: manages options for dealing and fixing malformed imported files
//---------------------------------------------------------------------------------------
/**
Class MusicXmlOptions stores and manages the settings for dealing and fixing malformed imported files.
The only instance of this class is stored in the %LibraryScope object. You can access
it and modify its settings either from %LomseDoorway object or from the %LibraryScope object:

@code
	LomseDoorway* pLomse = ...
	MusicXmlOptions* opt = pLomse->get_musicxml_options();

@endcode

@code
	LibraryScope& lomse = ...
	MusicXmlOptions* opt = lomse.get_musicxml_options();

@endcode

Example of use:

@code
	LomseDoorway* pLomse = ...
	MusicXmlOptions* opt = pLomse->get_musicxml_options();
	opt->fix_beams(true);
	Presenter* pPresenter = pLomse->open_document(ViewFactory::k_view_vertical_book,
												  "my_score.xml");
	...
	opt->fix_beams(false);
	Presenter* pPresenter = pLomse->open_document(ViewFactory::k_view_vertical_book,
												  "other_score.xml");

@endcode


Settings description and default values for Lomse library are described in the following table:

<table>
<tr><th>Setting</th>        <th>Default value</th>  <th>Description</th>
<tr><td>fix_beams</td>		<td>true</td>
	<td>When %true, if beam information is not congruent with note type, the importer will fix the beam.</td></tr>
<tr><td>use_default_clefs</td>		<td>true</td>
	<td>When %true, if an score part has pitched notes but the clef is missing, the importer will assume a G or an F4 clef, depending on notes pitch range.</td></tr>
</table>

@see fix_beams(), use_default_clefs()
*/
class MusicXmlOptions
{
public:

    /** Sets the value for 'fix_beams' option. When %true, if beam information is not congruent with note type,
		the importer will fix the beam.
	*/ 
    void fix_beams(bool value);

    /** Sets the value for 'use_default_clefs' option. When %true, if an score part has pitched notes but the 
		clef is missing, the importer will assume a G or an F4 clef, depending on notes pitch range.
	*/ 
    void use_default_clefs(bool value);

	/**
	Returns current setting for the 'fix_beams' option.
	*/
    bool fix_beams();
	/**
	Returns current setting for the 'use_default_clefs' option.
	*/
    bool use_default_clefs();

};

