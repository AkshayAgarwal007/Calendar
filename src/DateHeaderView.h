/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _DATEHEADERVIEW_H_
#define _DATEHEADERVIEW_H_

#include <DateTime.h>
#include <View.h>


class BStringView;


class DateHeaderView: public BView {
public:
					DateHeaderView();
		void 			MessageReceived(BMessage* message);
		void 			UpdateDateHeader(const BDate& date
						= BDate::CurrentDate(B_LOCAL_TIME));
private:
		BStringView*		fDayLabel;
		BStringView*		fDayOfWeekLabel;
		BStringView*		fMonthYearLabel;
};

#endif
