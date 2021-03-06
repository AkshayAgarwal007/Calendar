/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H


#include <DateTime.h>
#include <Messenger.h>
#include <Window.h>


class BBox;
class BButton;
class BCheckBox;
class BList;
class BMenu;
class BMenuField;
class BPopUpMenu;
class BRadioButton;
class BStringView;
class BTextControl;
class BTextView;
class BView;
class Category;
class Event;
class Preferences;
class SQLiteManager;
class TDateEdit;
class TTimeEdit;


static const uint32 kEventWindowQuitting = 'kewq';
static const uint32 kShowPopUpCalendar = 'kspc';
static const uint32 kStartDateChanged = 'ksdc';
static const uint32 kEndDateChanged = 'kedc';


class EventWindow: public BWindow {
public:
				EventWindow();
				~EventWindow();

	virtual void		MessageReceived(BMessage* message);
	virtual bool		QuitRequested();
	virtual void		FrameMoved(BPoint newPosition);

	void			SetEvent(Event* event);
	void			SetEventDate(BDate& date);

	static void		SetPreferences(Preferences* preferences);

	void			OnCheckBoxToggle();
	void			OnSaveClick();
	void			OnDeleteClick();
	void			CloseWindow();

	void			GetDateFromMessage(BMessage* message,
					BDate& date);
	void			GetTimeFromMessage(BMessage* message,
					BTime& time);
private:
	void			_InitInterface();
	void 			_DisableControls();
	void			_UpdateCategoryMenu();
	void			_ShowPopUpCalendar(int8 which);

	static const uint32	kDeletePressed		= 1000;
	static const uint32	kCancelPressed		= 1001;
	static const uint32	kSavePressed		= 1002;
	static const uint32	kAllDayPressed		= 1003;
	static const uint32	kOptEveryMonth		= 1004;
	static const uint32	kOptEveryYear		= 1005;
	static const uint32 	kStartDateEditChanged	= 1006;
	static const uint32 	kEndDateEditChanged	= 1007;
	static const uint32 	kStartTimeEditChanged	= 1008;
	static const uint32 	kEndTimeEditChanged	= 1009;

	static Preferences*	fPreferences;

	BTextControl*		fTextName;
	BTextControl*		fTextPlace;

	TTimeEdit*		fStartTimeEdit;
	TTimeEdit*		fEndTimeEdit;
	TDateEdit*		fStartDateEdit;
	TDateEdit*		fEndDateEdit;

	BTextView*		fTextDescription;
	BView*			fMainView;

	BMenu*			fCategoryMenu;
	BMenuField*		fCategoryMenuField;

	BStringView*		fNameLabel;
	BStringView*		fPlaceLabel;
	BStringView*		fDescriptionLabel;
	BStringView*		fCategoryLabel;
	BStringView*		fAllDayLabel;
	BStringView*		fStartDateLabel;
	BStringView*		fStartTimeLabel;
	BStringView*		fEndDateLabel;
	BStringView*		fEndTimeLabel;

	BButton*		fDeleteButton;
	BButton*		fStartCalButton;
	BButton*		fEndCalButton;

	BRadioButton*		fEveryMonth;
	BRadioButton*		fEveryYear;

	BCheckBox*		fAllDayCheckBox;

	BBox*			fStartDateBox;
	BBox*			fEndDateBox;

	BMessenger		fCalendarWindow;

	BDate			fStartDate;
	BDate			fEndDate;

	Event*			fEvent;
	BList*			fCategoryList;

	SQLiteManager*		fDBManager;
};

#endif
