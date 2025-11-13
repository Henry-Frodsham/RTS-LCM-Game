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

struct OverlayEditPanelEvent {
	std::string NameOfExisting;
	std::string OverlayToFindIn;

	std::vector<float> NewPosition;
	std::vector<float> NewDimensions;

	std::string NewMaterialName;

	OverlayEditPanelEvent(std::string Name
		, std::string OverlayToUse
		, std::vector<float> Dimensions = {-1.f,-1.f}
		, std::vector<float> Position = {-1.f,-1.f}
		, std::string MaterialName = "USE_OLD")
		: NewPosition(Position)
		, NewDimensions(Dimensions)
		, NewMaterialName(MaterialName)
		, OverlayToFindIn(OverlayToUse)
		, NameOfExisting(Name)
	{
	}
};

struct OverlayEditTextEvent {
	std::string NameOfExisting;
	std::string OverlayToFindIn;

	std::string NewText;

	std::vector<float> NewPosition;
	std::vector<float> NewDimensions;

	std::string NewMaterialName;

	OverlayEditTextEvent(std::string Name
		, std::string OverlayToUse
		, std::vector<float> Position = {-1.f,-1.f}
		, std::vector<float> Dimensions = {-1.f,-1.f}
		, std::string MaterialName = "USE_OLD"
		, std::string Text = "USE_OLD")
		: NewPosition(Position)
		, NewDimensions(Dimensions)
		, NewMaterialName(MaterialName)
		, OverlayToFindIn(OverlayToUse)
		, NewText(Text)
		, NameOfExisting(Name)
	{
	}
};