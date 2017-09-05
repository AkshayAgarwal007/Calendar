/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2010-2017 Humdinger, humdingerb@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <Application.h>
#include <ControlLook.h>
#include <Font.h>
#include <MenuItem.h>

#include "EventListItem.h"


EventListItem::EventListItem(BString name, BString timeText, rgb_color color)
	:
	BListItem()
{
	fName = name;
	fTimeText = timeText;
	fColor = color;
}


EventListItem::~EventListItem()
{
}


void
EventListItem::DrawItem(BView* view, BRect rect, bool complete)
{
	float spacing = be_control_look->DefaultLabelSpacing();
	float offset = spacing;
	BFont headerFont;
	BFont footerFont;
	font_height finfo;

	rgb_color bgColor;
	if (IsSelected())
		bgColor = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	else
		bgColor = ui_color(B_LIST_BACKGROUND_COLOR);

	view->SetHighColor(bgColor);
	view->SetLowColor(bgColor);
	view->FillRect(rect);

	headerFont.SetSize(headerFont.Size() + 4);
	headerFont.GetHeight(&finfo);
	view->SetFont(&headerFont);

	// category indicator

	BRect colorRect(rect);
	colorRect.left += spacing + 2;
	colorRect.right = colorRect.left + rect.Height() / 6;
	colorRect.top = rect.top + ((rect.Height()
		- (finfo.ascent + finfo.descent + finfo.leading)) / 2);
		- headerFont.Size();

	colorRect.bottom = colorRect.top + rect.Height() / 6;
	view->SetHighColor(fColor);
	view->FillEllipse(colorRect);
	view->SetHighUIColor(B_CONTROL_BORDER_COLOR);
	view->StrokeEllipse(colorRect);
	offset += spacing * 2 + colorRect.Width();

	// name

	if (IsSelected())
		view->SetHighColor(tint_color(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR),
			0.9));
	else
		view->SetHighColor(tint_color(ui_color(B_LIST_ITEM_TEXT_COLOR), 0.9));


	view->MovePenTo(offset, rect.top + ((rect.Height()
		- (finfo.ascent + finfo.descent + finfo.leading)) / 2)
		+ (finfo.ascent + finfo.descent) - headerFont.Size() + 2 + 3);

	view->TruncateString(&fName, B_TRUNCATE_MIDDLE,  rect.Width() - offset - 2);
	view->DrawString(fName.String());

	// time period

	if (IsSelected())
		view->SetHighColor(tint_color(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR),
			0.7));
	else
		view->SetHighColor(tint_color(ui_color(B_LIST_ITEM_TEXT_COLOR), 0.6));

	footerFont.SetSize(headerFont.Size() - 2);
	footerFont.GetHeight(&finfo);
	view->SetFont(&footerFont);

	view->MovePenTo(offset,
		rect.top + headerFont.Size() - footerFont.Size() + 6 + ((rect.Height()
		- (finfo.ascent + finfo.descent + finfo.leading)) / 2)
		+ (finfo.ascent + finfo.descent));

		view->TruncateString(&fTimeText, B_TRUNCATE_MIDDLE, rect.Width() - offset - 2);
		view->DrawString(fTimeText.String());

	// draw lines

	view->SetHighColor(tint_color(ui_color(B_CONTROL_BACKGROUND_COLOR),
		B_DARKEN_1_TINT));
	view->StrokeLine(rect.LeftBottom(), rect.RightBottom());
}


void
EventListItem::Update(BView* owner, const BFont* finfo)
{
	// we need to override the update method so we can make sure the
	// list item size doesn't change
	BListItem::Update(owner, finfo);

	float spacing = be_control_look->DefaultLabelSpacing();
	SetHeight(fItemHeight + spacing * 2);
}
