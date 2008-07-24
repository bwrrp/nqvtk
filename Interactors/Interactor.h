#pragma once

#include <QMouseEvent>

#include <QObject> // for qDebug
#include <cassert>

namespace NQVTK
{
	class Interactor
	{
	public:
		Interactor() { }

		// TODO: we might want to make this independent of Qt some day
		virtual bool MouseMoveEvent(QMouseEvent *event)
		{
			return false
		}

	private:
		// Not implemented
		Interactor(const Interactor&);
		void operator=(const Interactor&);
	};
}