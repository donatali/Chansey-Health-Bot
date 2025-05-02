import { StyleSheet } from "react-native";
import { COLORS, FONT, SHADOWS, SIZES } from "../../../../constants";

const styles = StyleSheet.create({
  container: (selected, user, color = COLORS.primary) => ({
    width: 160,
    height: 180,
    padding: SIZES.medium,
    backgroundColor: selected === user?.id ? color : "#FFF",
    borderRadius: 10,
    borderWidth: 2,
    borderColor: "black",
    ...SHADOWS.medium,
    shadowColor: COLORS.black,
    marginHorizontal: SIZES.small,
    marginVertical: SIZES.small,
  }),
  cubeContainer: {
    flex: 1,
    justifyContent: "space-between",
  },
  cubeFace: (selected, user, color = COLORS.primary) => ({
    width: "100%",
    height: 100, // Adjusted for better proportions
    padding: SIZES.small,
    backgroundColor: selected === user?.id ? "#FFFFFF" : color === COLORS.primary ? COLORS.lightWhite : color + '22', // Add transparency
    borderRadius: 8,
    borderWidth: 1,
    borderColor: "black",
    justifyContent: "center",
    alignItems: "center",
    overflow: "hidden",
    ...SHADOWS.medium,
    elevation: 5,
  }),
  userImage: {
    width: "90%",  // Fill the container width
    height: "90%", // Fill the container height
    alignSelf: "center",
    resizeMode: "contain", // Scale to fit without cropping
  },
  overlay: {
    ...StyleSheet.absoluteFillObject,
    backgroundColor: "rgba(0,0,0,0.1)", // lighter overlap
    borderRadius: 8,
  },
  infoContainer: {
    marginTop: SIZES.small,
  },
  userName: (selected, user, color = COLORS.primary) => ({
    fontSize: SIZES.medium + 2,
    fontFamily: "PokemonFont",
    color: selected === user?.id ? COLORS.white : COLORS.black,
    textAlign: "center",
    marginBottom: 4,
  }),
  infoWrapper: {
    flexDirection: "row",
    marginTop: 4,
    justifyContent: "center",
    alignItems: "center",
  },
  statsPreview: {
    marginTop: 2,
    alignItems: "center",
    backgroundColor: selected => selected ? "rgba(255,255,255,0.2)" : "rgba(0,0,0,0.05)",
    borderRadius: 4,
    paddingVertical: 2,
    paddingHorizontal: 4,
  },
  statsText: (selected, user, color = COLORS.primary) => ({
    fontSize: SIZES.small + 1,
    fontFamily: "PokemonFont",
    color: selected === user?.id ? COLORS.white : COLORS.gray,
    textAlign: "center",
  }),
});

export default styles;

