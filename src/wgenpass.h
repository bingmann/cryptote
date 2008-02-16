// -*- C++ -*- generated by wxGlade 0.6.2 on Fri Feb 15 21:27:17 2008
// $Id$

#include <wx/wx.h>
#include <wx/image.h>
// begin wxGlade: ::dependencies
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/listctrl.h>
// end wxGlade

#include <vector>

#ifndef WGENPASS_H
#define WGENPASS_H

// begin wxGlade: ::extracode
// end wxGlade

class WGeneratePassword : public wxDialog
{
public:
    // begin wxGlade: WGeneratePassword::ids
    enum {
        myID_PRESET = wxID_HIGHEST + 1000,
        myID_PRESET_ADD = wxID_HIGHEST + 1002,
        myID_PRESET_REMOVE = wxID_HIGHEST + 1004,
        myID_TYPE = wxID_HIGHEST + 1006,
        myID_SKIPSIMILARCHARS = wxID_HIGHEST + 1008,
        myID_SKIPSWAPPEDCHARS = wxID_HIGHEST + 1010,
        myID_LENGTH = wxID_HIGHEST + 1012,
        myID_ENUMERATE = wxID_HIGHEST + 1014,
        myID_GENERATE = wxID_HIGHEST + 1016,
        myID_PASSLIST = wxID_HIGHEST + 1018
    };
    // end wxGlade

    WGeneratePassword(wxWindow* parent, bool standalone, int id=wxID_ANY, const wxString& title=wxEmptyString, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_DIALOG_STYLE);

    /// Set if running as stand-alone program.
    bool	standalone;

    /// Return password selected by the user
    const wxString&	GetSelectedPassword() const;

protected:
    // *** Preset Management ***

    struct Preset
    {
	wxString	name;
	int		type;
	bool		skipsimilar;
	bool		skipswapped;
	int		length;
	bool		enumerate;
    };

    typedef std::vector<Preset> presetlist_type;

    /// List of all password presets
    std::vector<Preset> presetlist;

    /// Save selected password for calling dialog
    wxString		selpass;

    // *** Helper Functions ***

    void		ResetPresetChoice();

    /// Update the keybits text control
    void		UpdateKeyStrength();

    /// Update disable/enable status of the check boxes
    void		UpdateCheckboxes();

    /// (Re)Generate Password list
    void		GenerateList();

    /// Save current settings to config file/registry
    void		SaveSettings();

    // *** Password Generator Functions ***

    /// Password generator types
    enum pass_type {
	PT_PRONOUNCEABLE,
	PT_ALPHANUMERIC,
	PT_ALPHA,
	PT_ALPHALOWER,
	PT_ALPHAUPPER,
	PT_HEXADECIMAL,
	PT_NUMERIC,
	PT_PORTNUMBER,
	PT_LAST = PT_PORTNUMBER
    };

    /// Return ASCII Name for password generator type
    static const wxChar* GetTypeName(pass_type pt);

    /// Make one password of the generic random sequence type.
    static wxString	MakePasswordType0(unsigned int len, const wxChar* letters);

    /// Return array of possible letters in simple random password.
    static const wxChar* GetType0Letters(pass_type pt, bool skip_similar, bool skip_swapped);

    /// Return keybits per letter for currently selected generator type;
    float		GetTypeKeybits() const;

    /// Return true if the options skip similar characters is available with
    /// the selected generator type.
    bool		IsAllowedSimilar() const;

    /// Return true if the options skip swapped characters is available with
    /// the selected generator type.
    bool		IsAllowedSwapped() const;

    /// Return true if the spinctrl length is available with the selected
    /// generator type.
    bool		IsAllowedLength() const;

    /// Make one password of the currently set type.
    wxString		MakePassword(unsigned int passlen);

private:
    // begin wxGlade: WGeneratePassword::methods
    void set_properties();
    void do_layout();
    // end wxGlade

protected:
    // begin wxGlade: WGeneratePassword::attributes
    wxStaticBox* sizer2_staticbox;
    wxChoice* choicePreset;
    wxBitmapButton* buttonPresetAdd;
    wxBitmapButton* buttonPresetRemove;
    wxChoice* choiceType;
    wxCheckBox* checkboxSkipSimilarChars;
    wxCheckBox* checkboxSkipSwappedChars;
    wxSpinCtrl* spinctrlLength;
    wxTextCtrl* textctrlStrength;
    wxCheckBox* checkboxEnumerate;
    wxButton* buttonGenerate;
    wxListCtrl* listctrlPasslist;
    wxTextCtrl* textctrlPasslist;
    wxButton* buttonOK;
    wxButton* buttonCancel;
    wxButton* buttonClose;
    wxButton* buttonAbout;
    // end wxGlade

    DECLARE_EVENT_TABLE();

public:
    virtual void OnClose(wxCloseEvent& event);
    virtual void OnChoicePreset(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void OnChoiceType(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void OnCheckSkipSimilarChars(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void OnCheckSkipSwappedChars(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void OnSpinLength(wxSpinEvent &event); // wxGlade: <event_handler>
    virtual void OnCheckEnumerate(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void OnButtonGenerate(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void OnPasslistSelected(wxListEvent &event); // wxGlade: <event_handler>
    virtual void OnPasslistActivated(wxListEvent &event); // wxGlade: <event_handler>
    virtual void OnButtonOK(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void OnButtonCancel(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void OnButtonClose(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void OnButtonAbout(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void OnButtonPresetAdd(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void OnButtonPresetRemove(wxCommandEvent &event); // wxGlade: <event_handler>
}; // wxGlade: end class

#endif // WGENPASS_H
