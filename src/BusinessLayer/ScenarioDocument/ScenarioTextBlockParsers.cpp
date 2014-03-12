#include "ScenarioTextBlockParsers.h"

#include <QString>
#include <QStringList>
#include <QRegularExpression>

using namespace BusinessLogic;


QString CharacterParser::name(const QString& _text)
{
	//
	// В блоке персонажа так же могут быть указания, что он говорит за кадром и т.п.
	// эти указания даются в скобках
	//

	QString name = _text;
	return name.remove(QRegularExpression("[(](.*)[)]")).simplified();
}

// ****

TimeAndPlaceParser::Section TimeAndPlaceParser::section(const QString& _text)
{
	TimeAndPlaceParser::Section section = SectionUndefined;

	if (_text.split(", ").count() == 2) {
		section = SectionScenarioDay;
	} else if (_text.split(" - ").count() == 2) {
		section = SectionTime;
	} else {
		const int splitDotCount = _text.split(". ").count();
		if (splitDotCount == 1) {
			section = SectionPlace;
		} else {
			section = SectionLocation;
		}
	}

	return section;
}

QString TimeAndPlaceParser::placeName(const QString& _text)
{
	QString placeName;

	if (_text.split(". ").count() > 0) {
		placeName = _text.split(". ").value(0);
	}

	return placeName;
}

QString TimeAndPlaceParser::locationName(const QString& _text)
{
	QString locationName;

	if (_text.split(". ").count() > 1) {
		locationName = _text.mid(_text.indexOf(". ") + 2);
		locationName = locationName.split(" - ").first();
	}

	return locationName;
}

QString TimeAndPlaceParser::scenarioDayName(const QString& _text)
{
	QString scenarioDayName;

	if (_text.split(", ").count() == 2) {
		scenarioDayName = _text.split(", ").last();
	}

	return scenarioDayName;
}

QString TimeAndPlaceParser::timeName(const QString& _text)
{
	QString timeName;

	if (_text.split(" - ").count() == 2) {
		timeName = _text.split(" - ").last().split(",").first();
	}

	return timeName;
}