#pragma once


struct OverlayAddBoxEvent {
	std::vector<float> Position;
	std::vector<float> Dimensions;

	std::string Name;
	std::string MaterialName;
	std::string OverlayToUse;

	OverlayAddBoxEvent(std::vector<float> Position
					, std::vector<float> Dimensions
					, std::string Name
					, std::string MaterialName
					, std::string OverlayToUse)
					: Position(Position)
					, Dimensions(Dimensions)
					, Name(Name)
					, MaterialName(MaterialName)
					, OverlayToUse(OverlayToUse)
	{}
};

struct OverlayAddTextEvent {
	std::vector<float> Position;
	std::vector<float> Dimensions;

	std::string Name;
	std::string Text;
	std::string MaterialName;
	std::string OverlayToUse;

	OverlayAddTextEvent(std::vector<float> Position
		, std::vector<float> Dimensions
		, std::string Name
		, std::string MaterialName
		, std::string OverlayToUse
		, std::string Text)
		: Position(Position)
		, Dimensions(Dimensions)
		, Name(Name)
		, MaterialName(MaterialName)
		, OverlayToUse(OverlayToUse)
		, Text(Text)
	{
	}
};