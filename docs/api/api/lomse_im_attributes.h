//-----------------------------------------------------------------------------
//attributes
/**
    @ingroup enumerations

    This enum describes values for valid attributes (for CmdChangeAttribute commands). 

    \#include <lomse_im_attributes.h>
*/
enum EImoAttribute
{
    k_attr_barline=0,               ///< Barline type
    k_attr_color,                   ///< Color value
    k_attr_dots,                    ///< Number of dots (dotted notes & rests)
    k_attr_notated_accidentals,     ///< Notated accidentals for notes
    k_attr_note_type,               ///< Note type
    k_attr_octave,                  ///< Octave (note pitch)
    k_attr_staff_num,               ///< Number of staff
    k_attr_stem_type,               ///< Stem type
    k_attr_step,                    ///< Step (note pitch)
    k_attr_style,                   ///< Style
    k_attr_time_modifier_bottom,    ///< Value for time modifier, bottom number (tuplets)
    k_attr_time_modifier_top,       ///< Value for time modifier, top number (tuplets)
    k_attr_visible,                 ///< Visible (display or hide the element)
    k_attr_voice,                   ///< Voice (for notes & rests)
    k_attr_xpos,                    ///< X position (to place an element)
    k_attr_ypos,                    ///< Y position (to place an element)

    k_attr_coda,                //string
    k_attr_dacapo,              //bool
    k_attr_dalsegno,            //string
    k_attr_dynamics,            //float
    k_attr_fine,                //bool
    k_attr_forward_repeat,      //bool
    k_attr_pizzicato,           //bool
    k_attr_segno,               //string
    k_attr_tempo,               //float
    k_attr_time_only,           //string
    k_attr_tocoda,              //string
};

//data types
/**
    @ingroup enumerations

    This enum describes data type for an attribute.

    \#include <lomse_im_attributes.h>
*/
enum EDataType
{
    k_type_bool=0,      ///< Boolean
    k_type_color,       ///< Color
    k_type_double,      ///< Float number, double precission
    k_type_int,         ///< Integer number
    k_type_string,      ///< String of characters
};

/*
//---------------------------------------------------------------------------------------
// Fully describes an attribute
struct AttributesData
{
    int         attrb;          //referred attribute
    EDataType   type;           //data type
    string      label;          //short name, to be used as lablel in dialogs
    string      description;    //description, for tooltips or other
    string      units;          //units or empty
    int         minimum;        //minimum allowed value (for EDataType == k_type_int)
    int         maximum;        //maximum allowed value (for EDataType == k_type_int)

    AttributesData(int a, EDataType t, const string& lbl,
                   const string& descr)
        : attrb(a)
        , type(t)
        , label(lbl)
        , description(descr)
        , units("")
        , minimum(0)
        , maximum(0)
    {
    }
    AttributesData(int a, EDataType t, const string& lbl,
                   const string& descr, string u, int mn, int mx)
        : attrb(a)
        , type(t)
        , label(lbl)
        , description(descr)
        , units(u)
        , minimum(mn)
        , maximum(mx)
    {
    }

};

//---------------------------------------------------------------------------------------
// provides information about existing attributes
class AttributesTable
{
protected:

public:
    AttributesTable() {}
	~AttributesTable() {}

    static AttributesData get_data_for(int attrb);

};

*/
