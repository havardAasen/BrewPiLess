export enum LineIndex {
    BeerSet = 0,
    BeerTemp = 1,
    FridgeTemp = 2,
    FridgeSet = 3,
    RoomTemp = 4,
    AuxTemp = 5,
    Gravity = 6,
    FilteredSg = 7,
}

export const ModeMap = {
    b: "Beer Constant",
    f: "Fridge Constant",
    o: "Off",
    p: "Profile",
} as const;

export const Colors = [
    "rgb(240, 100, 100)",
    "rgb(41,170,41)",
    "rgb(89, 184, 255)",
    "rgb(255, 161, 76)",
    "#AAAAAA",
    "#f5e127",
    "rgb(153,0,153)",
    "#000abb",
] as const;

export const Labels = [
    "Time",
    "beerSet",
    "beerTemp",
    "fridgeTemp",
    "fridgeSet",
    "roomTemp",
    "auxTemp",
    "gravity",
    "filtersg",
] as const;

export const ClassLabels = [
    "",
    "beer-set",
    "beer-temp",
    "fridge-temp",
    "fridge-set",
    "room-temp",
    "aux-temp",
    "gravity",
    "filtersg",
] as const;
