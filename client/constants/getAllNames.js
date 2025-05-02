import mockData from "./trainerData";

const allNames = Object.values(mockData)
  .flat()
  .map((trainer) => trainer.name.toLowerCase());

export default allNames;