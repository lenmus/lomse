//=====================================================================================
// wxMidi: A MIDI interface based on PortMidi, the Portable Real-Time MIDI Library
// --------------------------------------------------------------------------------
//
// Author:      Cecilio Salmeron
// Copyright:   (c) 2005-2011 Cecilio Salmeron
// Licence:     wxWidgets licence, version 3.1 or later at your choice.
//=====================================================================================
#ifdef __GNUG__
#pragma interface "wxMidi.cpp"
#endif

#ifndef __WXMIDI_H__		//to avoid nested includes
#define __WXMIDI_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

/*
	To do, when a new version of portmidi:
	1. Review wxMidiError enumeration (copied and adapted from portmidi.h)
	2. Review filter codes taken from portmidi.h
*/

// MIDI support throgh Portmidi lib
#include "portmidi.h"
#include "porttime.h"


//Constants
#define wxMIDI_VERSION		_T("1.5")
#define wxMidiDeviceID		PmDeviceID
#define wxMidiTimestamp		PmTimestamp
#define wxMidiPmMessage		PmMessage
#define wxMidiPmEvent		PmEvent



typedef enum {
	wxMIDI_NO_ERROR = 0,

	//error codes from portmidi. Name changed from pmXxxx to wxMIDI_ERROR_Xxxx
    wxMIDI_ERROR_HostError = -10000,
    wxMIDI_ERROR_InvalidDeviceId, /* out of range or output device when input is requested or vice versa */
    wxMIDI_ERROR_InsufficientMemory,
    wxMIDI_ERROR_BufferTooSmall,
    wxMIDI_ERROR_BufferOverflow,
    wxMIDI_ERROR_BadPtr,
    wxMIDI_ERROR_BadData, /* illegal midi data, e.g. missing EOX */
    wxMIDI_ERROR_InternalError,
    wxMIDI_ERROR_BufferMaxSize, /* buffer is already as large as it can be */

	//Additional error codes not in portmidi
	wxMIDI_ERROR_AlreadyListening,
	wxMIDI_ERROR_CreateThread,
	wxMIDI_ERROR_StartThread,
	wxMIDI_ERROR_BadSysExMsg_Start,
	wxMIDI_ERROR_BadSysExMsg_Length,
	wxMIDI_ERROR_NoDataAvailable
} wxMidiError;


//Macros
//----------------------------------------------------------------------------------

//macro Pm_Channel renamed as wxMIDI_CHANNEL
/*
    Pm_SetChannelMask() filters incoming messages based on channel.
    The mask is a 16-bit bitfield corresponding to appropriate channels
    The Pm_Channel macro can assist in calling this function.
    i.e. to set receive only input on channel 1, call with
    Pm_SetChannelMask(Pm_Channel(1));
    Multiple channels should be OR'd together, like
    Pm_SetChannelMask(Pm_Channel(10) | Pm_Channel(11))

    All channels are allowed by default
*/
#define wxMIDI_CHANNEL(channel) (1<<(channel))

// Filter codes
#define wxMIDI_NO_FILTER					0x00
#define wxMIDI_FILT_ACTIVE					PM_FILT_ACTIVE
#define wxMIDI_FILT_SYSEX					PM_FILT_SYSEX
#define wxMIDI_FILT_CLOCK					PM_FILT_CLOCK
#define wxMIDI_FILT_PLAY					PM_FILT_PLAY
#define wxMIDI_FILT_F9						PM_FILT_F9
#define wxMIDI_FILT_TICK					PM_FILT_TICK
#define wxMIDI_FILT_FD						PM_FILT_FD
#define wxMIDI_FILT_UNDEFINED				PM_FILT_UNDEFINED
#define wxMIDI_FILT_RESET					PM_FILT_RESET
#define wxMIDI_FILT_REALTIME				PM_FILT_REALTIME
#define wxMIDI_FILT_NOTE					PM_FILT_NOTE
#define wxMIDI_FILT_CHANNEL_AFTERTOUCH		PM_FILT_CHANNEL_AFTERTOUCH
#define wxMIDI_FILT_POLY_AFTERTOUCH			PM_FILT_POLY_AFTERTOUCH
#define wxMIDI_FILT_AFTERTOUCH				PM_FILT_AFTERTOUCH
#define wxMIDI_FILT_PROGRAM					PM_FILT_PROGRAM
#define wxMIDI_FILT_CONTROL					PM_FILT_CONTROL
#define wxMIDI_FILT_PITCHBEND				PM_FILT_PITCHBEND
#define wxMIDI_FILT_MTC						PM_FILT_MTC
#define wxMIDI_FILT_SONG_POSITION			PM_FILT_SONG_POSITION
#define wxMIDI_FILT_SONG_SELECT				PM_FILT_SONG_SELECT
#define wxMIDI_FILT_TUNE					PM_FILT_TUNE
#define wxMIDI_FILT_SYSTEMCOMMON			PM_FILT_SYSTEMCOMMON


//Declare a new command event to inform that MIDI input data is available
DECLARE_EVENT_TYPE(wxEVT_MIDI_INPUT, -1)

enum wxMidiMsgType
{
	wxMIDI_UNDEFINED_MSG = 0,
	wxMIDI_SHORT_MSG,
	wxMIDI_SYSEX_MSG
};


//forward declarations
class wxMidiThread;
class wxControlWithItems;


class wxMidiMessage
{
public:
	wxMidiMessage() { m_type = wxMIDI_UNDEFINED_MSG; }
	virtual ~wxMidiMessage() {}

	virtual void SetTimestamp(wxMidiTimestamp timestamp) = 0;
	virtual wxMidiTimestamp GetTimestamp() = 0;

	wxMidiMsgType GetType() { return m_type; }
	virtual wxByte GetStatus() = 0;


protected:
	wxMidiMsgType	m_type;

};


class wxMidiShortMessage : public wxMidiMessage
{
public:
	wxMidiShortMessage(wxByte status, wxByte data1, wxByte data2)
		: wxMidiMessage()
		{
			m_type = wxMIDI_SHORT_MSG;
			m_buffer.message = (((data2) << 16) & 0xFF0000) |
						(((data1) << 8) & 0xFF00) |
						((status) & 0xFF);
			m_buffer.timestamp = 0;
		}
	~wxMidiShortMessage() {}

	//timestamp
	void SetTimestamp(wxMidiTimestamp timestamp) { m_buffer.timestamp = timestamp; }
	wxMidiTimestamp GetTimestamp() { return m_buffer.timestamp; }

	// message data
	wxByte GetStatus() { return ((m_buffer.message) & 0xFF); }
	wxByte GetData1() { return (((m_buffer.message) >> 8) & 0xFF); }
	wxByte GetData2() { return (((m_buffer.message) >> 16) & 0xFF); }

	//internal usage
	PmEvent* GetBuffer() { return &m_buffer; }

private:
	PmEvent m_buffer;
};

class wxMidiSysExMessage : public wxMidiMessage
{
public:
	wxMidiSysExMessage(wxByte* msg, wxMidiTimestamp timestamp=0);
	wxMidiSysExMessage();
	~wxMidiSysExMessage();

	//timestamp
	void SetTimestamp(wxMidiTimestamp timestamp) { m_timestamp = timestamp; }
	wxMidiTimestamp GetTimestamp() { return m_timestamp; }

	// message data
	wxByte GetStatus() { return *m_pMessage; }
	wxByte* GetMessage() { return m_pMessage; }

	// information
	wxMidiError Error() { return m_nError; }
	int Length() { return m_nSize; }

	//two steps construction
	void SetBuffer(wxByte* pBuffer) { m_pMessage = pBuffer; }
	void SetLength(int lenght) { m_nSize = lenght; }


private:
	wxByte*			m_pMessage;
	wxMidiTimestamp	m_timestamp;
	wxMidiError		m_nError;
	int				m_nSize;
};

class wxMidiDevice
{
public:

	wxMidiDevice(wxMidiDeviceID nDevice);
	virtual ~wxMidiDevice();

	wxMidiError Close() { return (wxMidiError)Pm_Close(m_stream); }

	//
	// Device information
	//
	const wxString DeviceName();
	const wxString InterfaceUsed();
	bool IsInputPort();
	bool IsOutputPort();

	//
	// Errors
	//
	int HasHostError() { return Pm_HasHostError(m_stream); }



protected:

	wxMidiDeviceID			m_nDevice;
	const PmDeviceInfo*		m_pInfo;
	PortMidiStream*			m_stream;

};

class wxMidiOutDevice : public wxMidiDevice
{
public:

	wxMidiOutDevice(wxMidiDeviceID nDevice) : wxMidiDevice(nDevice) {}
	~wxMidiOutDevice() {}

	wxMidiError Open(long latency, void *DriverInfo=NULL);

	//
	// Write operations
	//
	wxMidiError Write(wxMidiShortMessage* pMsg);
	wxMidiError Write(wxMidiSysExMessage* pMsg);
	wxMidiError Write(wxByte* msg, wxMidiTimestamp when=0);

	//TODO: remove this when the interface is more ellaborated
	wxMidiError Write(PmEvent *buffer, long length) {
		return (wxMidiError)Pm_Write(m_stream, buffer, length);
	}

	//
	// Very common channel voice commands
	//
	wxMidiError	NoteOn(int channel, int note, int velocity);
	wxMidiError	NoteOff(int channel, int note, int velocity);
	wxMidiError	ProgramChange(int channel, int instrument);
	wxMidiError AllSoundsOff();

	// miscellaneous
	wxMidiError Abort() { return (wxMidiError)Pm_Abort(m_stream); }


private:

};

class wxMidiInDevice : public wxMidiDevice
{
public:

	wxMidiInDevice(wxMidiDeviceID nDevice);
	~wxMidiInDevice();

	wxMidiError Open(void *DriverInfo = NULL);

	wxMidiError Read(wxMidiPmEvent *buffer, long* length);
	wxMidiMessage* Read(wxMidiError* pError);

	wxMidiError SetFilter(long filters )
					{ return (wxMidiError)Pm_SetFilter(m_stream, filters); }

	wxMidiError SetChannelMask(long mask )
					{ return (wxMidiError)Pm_SetChannelMask(m_stream, mask); }

	wxMidiError Poll() { return (wxMidiError)Pm_Poll(m_stream); }
	void Flush();

	wxMidiError StartListening(wxWindow* pWindow, unsigned long nPollingRate=50);
	wxMidiError StopListening();

private:
	bool MoveDataToSysExBuffer(PmMessage message);


	wxMidiThread*	m_pThread;		//thread for polling

	//buffer for sysex messages
	long	m_SizeOfSysexBuffer;
	wxByte*	m_SysexBuffer;
	wxByte*	m_CurSysexDataPtr;
	bool	m_fReadingSysex;		// sysex message interrupted by real-time message
	wxMidiTimestamp	m_timestamp;	// timestamp of the interrupted sysex message
	bool	m_fEventPending;		// sysex message ended without EOX. There is a PmEvent
									// .. read but not delivered
	PmEvent	m_event;				// event pending to be processed
};


//Database for Midi GM (General MIDI Standard)
class wxMidiDatabaseGM
{
public:
	~wxMidiDatabaseGM();

	static wxMidiDatabaseGM* GetInstance();

	void	 PopulateWithInstruments(wxControlWithItems* pCtrol, int nSection, int nInstr=0,
                                     bool fAddNumber=false);
	void	 PopulateWithPercusionInstr(wxControlWithItems* pCtrol, int iSel=0);
	int		 PopulateWithSections(wxControlWithItems* pCtrol, int nSelInstr=-1);
	void	 PopulateWithAllInstruments(wxControlWithItems* pCtrol, int nInstr=0);

	int		 GetNumSections();
	int		 GetNumInstrumentsInSection(int nSect);
	int		 GetInstrFromSection(int nSect, int i);
	wxString GetInstrumentName(int nInstr);
	wxString GetSectionName(int nSect);


private:
	wxMidiDatabaseGM();
	void	Initialize();

	#define NUM_SECTIONS	16			//number of sections
	#define NUM_INSTRS		8			//max. number of instruments per section
	int			m_nSectInstr[NUM_SECTIONS][NUM_INSTRS];		//instruments in each section
	int			m_nNumInstrInSection[NUM_SECTIONS];			//points to last instrument in section
	wxString	m_sSectName[NUM_SECTIONS];					//section names

	static wxMidiDatabaseGM*	m_pInstance;	//the only instance of this class

};

class wxMidiSystem
{
public:
	~wxMidiSystem();

	static wxMidiSystem* GetInstance();


	wxMidiTimestamp GetTime() { return Pt_Time(); }
	const wxString GetErrorText( wxMidiError errnum );
	wxString GetHostErrorText();

	int CountDevices() { return Pm_CountDevices(); }

protected:
	wxMidiSystem() {}

private:
	wxMidiError Initialize();
	wxMidiError Terminate();

	static wxMidiSystem*	m_pInstance;	//the only instance of this class
};


class wxMidiThread : public wxThread
{
public:
	wxMidiThread(wxMidiInDevice* pDev, wxWindow* pWindow, unsigned long milliseconds);
	~wxMidiThread();

    // thread execution starts here
    void *Entry();

    // called when the thread exits
	void OnExit() {}

public:
	wxMidiInDevice*	m_pDev;				//owner Midi device
	wxWindow*		m_pWindow;			//the window tha will receive the events
	unsigned long	m_nMilliseconds;	//Midi in polling interval, in milliseconds

};


#endif  // __WXMIDI_H__
