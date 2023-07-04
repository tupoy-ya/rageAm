#include "element.h"

ConstString rageam::xml::Element::FormatTemp(ConstString fmt, ...) const
{
	static char buffer[TEMP_BUFFER_SIZE];

	va_list args;
	va_start(args, fmt);
	vsprintf_s(buffer, TEMP_BUFFER_SIZE, fmt, args);
	va_end(args);
	return buffer;
}

rageam::xml::ElementChildIterator rageam::xml::Element::begin() const
{
	nElement nChild = GetChild(nullptr);
	if (!nChild.HasValue())
		return ElementChildIterator::End();

	Element child = nChild.GetValue();
	return ElementChildIterator(child);
}

rageam::xml::ElementChildIterator rageam::xml::Element::end() const
{
	return ElementChildIterator::End();
}
