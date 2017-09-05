/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "EventWindow.h"

#include <time.h>

#include <Application.h>
#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <DateFormat.h>
#include <File.h>
#include <GroupLayout.h>
#include <GraphicsDefs.h>
#include <LayoutItem.h>
#include <LayoutBuilder.h>
#include <List.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <SeparatorView.h>
#include <StringView.h>
#include <Screen.h>
#include <TextControl.h>
#include <TextView.h>
#include <TimeFormat.h>
#include <View.h>

#include "App.h"
#include "CalendarMenuWindow.h"
#include "Category.h"
#include "CategoryEditWindow.h"
#include "DateTimeEdit.h"
#include "Event.h"
#include "MainWindow.h"
#include "Preferences.h"
#include "SQLiteManager.h"


Preferences* EventWindow::fPreferences = NULL;


EventWindow::EventWindow()
	:
	BWindow(fPreferences->fEventWindowRect, "Event Manager", B_TITLED_WINDOW,
			B_AUTO_UPDATE_SIZE_LIMITS)
{
	_InitInterface();

	if (fPreferences->fEventWindowRect == BRect()) {
		fPreferences->fEventWindowRect = Frame();
		CenterOnScreen();
	}

	_DisableControls();
}


EventWindow::~EventWindow()
{
	delete fDBManager;
	delete fCategoryList;
}


void
EventWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		case kAllDayPressed:
			OnCheckBoxToggle();
			break;

		case kCancelPressed:
			PostMessage(B_QUIT_REQUESTED);
			break;

		case kDeletePressed:
			OnDeleteClick();
			break;

		case kSavePressed:
			OnSaveClick();
			break;

		case kRefreshCategoryList:
		{	LockLooper();
			_UpdateCategoryMenu();
			UnlockLooper();
			break;
		}

		case kShowPopUpCalendar:
		{
			int8 which;
			message->FindInt8("which", &which);
			_ShowPopUpCalendar(which);
			break;
		}

		case kStartDateChanged:
		{
			BDate date;
			GetDateFromMessage(message, date);
			fStartDateEdit->SetDate(date);
			fStartTimeEdit->SetDate(date);
			break;
		}

		case kEndDateChanged:
		{
			BDate date;
			GetDateFromMessage(message, date);
			fEndDateEdit->SetDate(date);
			fEndTimeEdit->SetDate(date);
			break;
		}

		case kStartDateEditChanged:
		{
			BDate date;
			GetDateFromMessage(message, date);
			fStartDateEdit->SetDate(date);
			fStartTimeEdit->SetDate(date);
			break;
		}

		case kEndDateEditChanged:
		{
			BDate date;
			GetDateFromMessage(message, date);
			fEndDateEdit->SetDate(date);
			fEndTimeEdit->SetDate(date);
			break;
		}

		case kStartTimeEditChanged:
		{
			BTime time;
			GetTimeFromMessage(message, time);
			fStartTimeEdit->SetTime(time.Hour(), time.Minute(), time.Second());
			break;
		}

		case kEndTimeEditChanged:
		{
			BTime time;
			GetTimeFromMessage(message, time);
			fEndTimeEdit->SetTime(time.Hour(), time.Minute(), time.Second());
			break;
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
EventWindow::FrameMoved(BPoint newPosition)
{
	fPreferences->fEventWindowRect.OffsetTo(newPosition);
}


void
EventWindow::SetEvent(Event* event)
{
	fEvent = event;

	if (event != NULL) {
		fTextName->SetText(event->GetName());
		fTextPlace->SetText(event->GetPlace());
		fTextDescription->SetText(event->GetDescription());

		fStartDateEdit->SetDate(BDate(event->GetStartDateTime()));
		fEndDateEdit->SetDate(BDate(event->GetEndDateTime()));

		fStartTimeEdit->SetTime_t(event->GetStartDateTime() - timezone);
		fEndTimeEdit->SetTime_t(event->GetEndDateTime() - timezone);

		Category* category;

		for (int32 i = 0; i < fCategoryList->CountItems(); i++) {
			category = ((Category*)fCategoryList->ItemAt(i));
			if (category->Equals(*event->GetCategory())) {
				fCategoryMenu->ItemAt(i)->SetMarked(true);
				break;
			}
		}

		if (event->IsAllDay()) {
			fAllDayCheckBox->SetValue(B_CONTROL_ON);
			fStartTimeEdit->SetEnabled(false);
			fEndTimeEdit->SetEnabled(false);
		} else
			fAllDayCheckBox->SetValue(B_CONTROL_OFF);

		fDeleteButton->SetEnabled(true);
	}
}


void
EventWindow::GetDateFromMessage(BMessage* message, BDate& date)
{
	int32 day, month, year;
	message->FindInt32("day", &day);
	message->FindInt32("month", &month);
	message->FindInt32("year", &year);
	date = BDate(year, month, day);
}


void
EventWindow::GetTimeFromMessage(BMessage* message, BTime& time)
{
	int32 hour, minute, second;
	message->FindInt32("hour", &hour);
	message->FindInt32("minute", &minute);
	message->FindInt32("second", &second);
	time = BTime(hour, minute, second);
}


void
EventWindow::SetEventDate(BDate& date)
{
	fStartDateEdit->SetDate(date);
	fEndDateEdit->SetDate(date);
	fStartTimeEdit->SetDate(date);
	fEndTimeEdit->SetDate(date);
	fStartTimeEdit->SetTime(0, 0, 0);
	fEndTimeEdit->SetTime(1, 0, 0);
}


bool
EventWindow::QuitRequested()
{
	((App*)be_app)->mainWindow()->PostMessage(kEventWindowQuitting);
	return true;
}


void
EventWindow::SetPreferences(Preferences* preferences)
{
	fPreferences = preferences;
}


void
EventWindow::OnSaveClick()
{
	if (BString(fTextName->Text()).CountChars() < 3) {

		BAlert* alert  = new BAlert("Error",
			"The name must have a length greater than 2.",
			NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

		alert->SetShortcut(0, B_ESCAPE);
		alert->Go();
		return;
	}

	time_t start;
	time_t end;

	BDate startDate;
	BDate endDate;

	BTime startTime;
	BTime endTime;

	startDate = fStartDateEdit->GetDate();
	endDate = fEndDateEdit->GetDate();

	startTime = fStartTimeEdit->GetTime();
	endTime = fEndTimeEdit->GetTime();

	start = BDateTime(startDate, startTime).Time_t();
	end = BDateTime(endDate, endTime).Time_t();

	if (difftime(start, end) > 0) {
		BAlert* alert  = new BAlert("Error",
			"Sorry, you cannot create an event that ends before it starts.",
			NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

		alert->SetShortcut(0, B_ESCAPE);
		alert->Go();
		return;
	}

	Category* category = NULL;
	BMenuItem* item = fCategoryMenu->FindMarked();
	int32 index = fCategoryMenu->IndexOf(item);
	Category* c = ((Category*)fCategoryList->ItemAt(index));
	category = new Category(*c);

	bool notified = (difftime(start, BDateTime::CurrentDateTime(B_LOCAL_TIME).Time_t()) < 0) ? true : false;

	Event newEvent(fTextName->Text(), fTextPlace->Text(),
		fTextDescription->Text(), fAllDayCheckBox->Value() == B_CONTROL_ON,
		start, end, category, notified);

	if ((fEvent == NULL) && (fDBManager->AddEvent(&newEvent))) {
		CloseWindow();
	} else if ((fEvent != NULL) && (fDBManager->UpdateEvent(fEvent, &newEvent))) {
		CloseWindow();
	} else {
		BAlert* alert  = new BAlert("Error",
			"There was some error in adding the event. Please try again.",
			NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
		return;
	}
}


void
EventWindow::OnDeleteClick()
{
	BAlert* alert = new BAlert("Confirm delete",
		"Are you sure you want to delete this event?",
		NULL, "OK", "Cancel", B_WIDTH_AS_USUAL, B_WARNING_ALERT);

	alert->SetShortcut(1, B_ESCAPE);
	int32 button_index = alert->Go();

	if (button_index == 0) {
		Event newEvent(*fEvent);
		newEvent.SetStatus(false);
		newEvent.SetUpdated(time(NULL));
		fDBManager->UpdateEvent(fEvent, &newEvent);
		CloseWindow();
	}
}


void
EventWindow::CloseWindow()
{
	PostMessage(B_QUIT_REQUESTED);
}


void
EventWindow::OnCheckBoxToggle()
{
	if (fAllDayCheckBox->Value() == B_CONTROL_ON) {
		fStartTimeEdit->SetEnabled(false);
		fEndTimeEdit->SetEnabled(false);
		fStartTimeEdit->SetTime(0, 0, 0);
		fEndTimeEdit->SetTime(23, 59, 59);
	} else {
		fStartTimeEdit->SetTime(0, 0, 0);
		fEndTimeEdit->SetTime(1, 0, 0);
		fStartTimeEdit->SetEnabled(true);
		fEndTimeEdit->SetEnabled(true);
	}
}


void
EventWindow::_InitInterface()
{
	fMainView = new BView("MainView", B_WILL_DRAW);
	fMainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fTextName = new BTextControl("EventName", NULL, NULL, NULL);
	fTextPlace = new BTextControl("EventPlace", NULL, NULL, NULL);

	fTextDescription = new BTextView("TextDescription", B_WILL_DRAW);
	fTextDescription->MakeEditable();
	fTextDescription->SetExplicitMinSize(BSize(240, 100));

	fAllDayCheckBox = new BCheckBox("", new BMessage(kAllDayPressed));
	fAllDayCheckBox->SetValue(B_CONTROL_OFF);

	fEveryMonth = new BRadioButton("EveryMonth", "Monthly", new BMessage(kOptEveryMonth));
	fEveryYear = new BRadioButton("EveryYear", "Yearly", new BMessage(kOptEveryYear));

	fNameLabel = new BStringView("NameLabel", "Name:");
	fPlaceLabel = new BStringView("PlaceLabel", "Place:");
	fDescriptionLabel = new BStringView("DescriptionLabel", "Description:");
	fCategoryLabel = new BStringView("CategoryLabel", "Category:");
	fAllDayLabel = new BStringView("AllDayLabel", "All Day:");
	fEndDateLabel = new BStringView("EndDateLabel", "End Date:");
	fStartDateLabel = new BStringView("StartDateLabel", "Start Date:");
	fStartTimeLabel = new BStringView("StartTimeLabel", "Start Time:");
	fEndTimeLabel = new BStringView("EndTimeLabel", "End Time:");

	fDeleteButton = new BButton("DeleteButton", "Delete", new BMessage(kDeletePressed));
	BButton* CancelButton = new BButton("CancelButton", "Cancel", new BMessage(kCancelPressed));
	BButton* SaveButton = new BButton("SaveButton", "OK", new BMessage(kSavePressed));

	BMessage* message = new BMessage(kShowPopUpCalendar);
	message->AddInt8("which", 0);
	fStartCalButton = new BButton("StartCalButton", "▼", message);
	message = new BMessage(kShowPopUpCalendar);
	message->AddInt8("which", 1);
	fEndCalButton = new BButton("EndCalButton", "▼", message);

	float width, height;
	fStartDateLabel->GetPreferredSize(&width, &height);
	fStartCalButton->SetExplicitMinSize(BSize(height * 2, height));
	fEndCalButton->SetExplicitMinSize(BSize(height * 2, height));

	fDBManager = new SQLiteManager();

	fCategoryList = fDBManager->GetAllCategories();

	fCategoryMenu = new BMenu("CategoryMenu");
	Category* category;
	for (int32 i = 0; i < fCategoryList->CountItems(); i++) {
		category = ((Category*)fCategoryList->ItemAt(i));
		fCategoryMenu->AddItem(new BMenuItem(category->GetName(),  B_OK));
	}

	fCategoryMenu->SetRadioMode(true);
	fCategoryMenu->SetLabelFromMarked(true);
	fCategoryMenu->ItemAt(0)->SetMarked(true);

	fStartDateEdit = new TDateEdit("Start date", 4, new BMessage(kStartDateEditChanged));
	fEndDateEdit = new TDateEdit("End date", 4, new BMessage(kEndDateEditChanged));
	fStartTimeEdit = new TTimeEdit("Start time", 4, new BMessage(kStartTimeEditChanged));
	fEndTimeEdit = new TTimeEdit("End time", 4, new BMessage(kEndTimeEditChanged));

	fCategoryMenuField = new BMenuField("CategoryMenuField", NULL, fCategoryMenu);

	BBox* fRecurrenceBox = new BBox("RecurrenceBox");
	BLayoutBuilder::Group<>(fRecurrenceBox, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_ITEM_INSETS)
		.AddStrut(B_USE_ITEM_SPACING)
		.AddGroup(B_HORIZONTAL)
			.Add(fEveryMonth)
			.Add(fEveryYear)
		.End()
	.End();
	fRecurrenceBox->SetLabel("Recurrence");

	fStartDateBox = new BBox("Start Date and Time");
	BLayoutBuilder::Group<>(fStartDateBox, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_ITEM_INSETS)
		.AddStrut(B_USE_ITEM_SPACING)
		.AddGrid()
			.Add(fStartDateLabel, 0, 0)
			.Add(fStartDateEdit, 1, 0)
			.Add(fStartCalButton, 2, 0)
			.Add(fStartTimeLabel, 0, 1)
			.Add(fStartTimeEdit, 1, 1)
		.End()
	.End();
	fStartDateBox->SetLabel("Start Date and Time");

	fEndDateBox = new BBox("End Date and Time");
	BLayoutBuilder::Group<>(fEndDateBox, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_ITEM_INSETS)
		.AddStrut(B_USE_ITEM_SPACING)
		.AddGrid()
			.Add(fEndDateLabel, 0, 0)
			.Add(fEndDateEdit, 1, 0)
			.Add(fEndCalButton, 2, 0)
			.Add(fEndTimeLabel, 0, 1)
			.Add(fEndTimeEdit, 1, 1)
		.End()
	.End();
	fEndDateBox->SetLabel("End Date and Time");

	BBox* divider = new BBox(BRect(0, 0, 1, 1),
		B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);
	divider->SetExplicitMaxSize(BSize(1, B_SIZE_UNLIMITED));

	BLayoutBuilder::Group<>(fMainView, B_HORIZONTAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_DEFAULT_SPACING, 0,
			B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
		.AddGroup(B_VERTICAL)
			.AddGrid()
				.Add(fNameLabel, 0, 0)
				.Add(fTextName, 1, 0)
				.Add(fPlaceLabel, 0, 1)
				.Add(fTextPlace, 1 ,1)
			.End()
			.Add(fDescriptionLabel)
			.Add(fTextDescription)
			.AddGrid()
				.Add(fCategoryLabel, 0, 0)
				.Add(fCategoryMenuField, 1, 0)
			.End()
		.End()
		.Add(divider)
		.AddGroup(B_VERTICAL)
			.AddGrid()
				.SetInsets(B_USE_ITEM_INSETS, B_USE_ITEM_INSETS,
					B_USE_ITEM_INSETS, 0)
				.Add(fAllDayLabel, 0, 0)
				.Add(fAllDayCheckBox, 1 ,0)
			.End()
			.Add(fStartDateBox)
			.Add(fEndDateBox)
			.Add(fRecurrenceBox)
		.End()
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(0, B_USE_DEFAULT_SPACING, 0, 0)
		.Add(fMainView)
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL)
			.SetInsets(B_USE_WINDOW_SPACING, B_USE_DEFAULT_SPACING,
				B_USE_DEFAULT_SPACING, B_USE_WINDOW_SPACING)
			.Add(fDeleteButton)
			.AddGlue()
			.Add(CancelButton)
			.Add(SaveButton)
		.End()
	.End();
}


void
EventWindow::_UpdateCategoryMenu()
{
	Category* selectedCategory = NULL;
	BMenuItem* item = fCategoryMenu->FindMarked();
	int32 index = fCategoryMenu->IndexOf(item);
	Category* c = ((Category*)fCategoryList->ItemAt(index));
	selectedCategory = new Category(*c);

	fCategoryList = fDBManager->GetAllCategories();

	Category* category;
	bool marked = false;

	fCategoryMenu->RemoveItems(0, fCategoryMenu->CountItems(), true);

	for (int32 i = 0; i < fCategoryList->CountItems(); i++) {
		category = ((Category*)fCategoryList->ItemAt(i));
		fCategoryMenu->AddItem(new BMenuItem(category->GetName(),  B_OK));
		if (category->Equals(*selectedCategory) && (marked == false)) {
			fCategoryMenu->ItemAt(i)->SetMarked(true);
			marked = true;
		}
	}

	if(!marked)
		fCategoryMenu->ItemAt(0)->SetMarked(true);

	delete selectedCategory;
}


void
EventWindow::_DisableControls()
{
	fEveryMonth->SetEnabled(false);
	fEveryYear->SetEnabled(false);
}


void
EventWindow::_ShowPopUpCalendar(int8 which)
{
	if (fCalendarWindow.IsValid()) {
		BMessage activate(B_SET_PROPERTY);
		activate.AddSpecifier("Active");
		activate.AddBool("data", true);

		if (fCalendarWindow.SendMessage(&activate) == B_OK)
			return;
	}

	BPoint where;
	BPoint boxPosition;
	BPoint buttonPosition;
	BDate date;
	BMessage* invocationMessage;

	//TODO: This is bad. Improve how coordinates for pop up calendar
	//window is calculated. Better implement a DateTimeEdit control.

	if (which == 0) {
		boxPosition = fStartDateBox->Frame().RightTop();
		buttonPosition = fStartCalButton->Frame().LeftBottom();
		date = fStartDateEdit->GetDate();
		invocationMessage = new BMessage(kStartDateChanged);

	} else {
		boxPosition = fEndDateBox->Frame().RightTop();
		buttonPosition = fEndCalButton->Frame().LeftBottom();
		date = fEndDateEdit->GetDate();
		invocationMessage = new BMessage(kEndDateChanged);
	}

	where.x = boxPosition.x - buttonPosition.x;
	where.y = boxPosition.y + buttonPosition.y;
	where += BPoint(-62.0, 8.0);

	ConvertToScreen(&where);

	CalendarMenuWindow* window = new CalendarMenuWindow(this, where);
	window->SetDate(date);
	window->SetInvocationMessage(invocationMessage);
	fCalendarWindow = BMessenger(window);
	window->Show();
}
