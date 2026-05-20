export const LineIndex = {
    BeerSet: 1,
    BeerTemp: 2,
    FridgeTemp: 3,
    FridgeSet: 4,
    RoomTemp: 5,
    AuxTemp: 6,
    Gravity: 7,
    FilteredSg: 8,
} as const;

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
