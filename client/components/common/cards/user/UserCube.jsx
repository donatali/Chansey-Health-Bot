import React, { useState } from "react";
import { View, Text, Image, TouchableOpacity, StyleSheet } from "react-native";
import { useRouter } from "expo-router";
import { useFonts } from "expo-font";

// Map of trainer names to their image files
const getTrainerImage = (trainerName) => {
  const trainerMap = {
    "Abraham": require("../../../../assets/trainers/Abraham.png"),
    "Alec": require("../../../../assets/trainers/Alec.png"),
    "Anna": require("../../../../assets/trainers/Anna.png"),
    "Ava": require("../../../../assets/trainers/Ava.png"),
    "James": require("../../../../assets/trainers/James.png"),
    "John": require("../../../../assets/trainers/John.png"),
    "Joseph": require("../../../../assets/trainers/Joseph.png"),
    "Junyi": require("../../../../assets/trainers/Junyi.png"),
    "Kris": require("../../../../assets/trainers/Kris.png"),
    "Matt": require("../../../../assets/trainers/Matt.png"),
    "Natalie": require("../../../../assets/trainers/Natalie.png"),
    "default": require("../../../../assets/trainers/Abraham.png")
  };

  return trainerName && trainerMap[trainerName] ? trainerMap[trainerName] : trainerMap.default;
};

// Array of colors for the cubes
const cubeColors = [
  '#4CAF50',  // Green
  '#FF5733',  // Red-orange
  '#3498DB',  // Blue
  '#9B59B6',  // Purple
  '#F1C40F',  // Yellow
  '#E74C3C',  // Red
  '#1ABC9C',  // Turquoise
  '#D35400',  // Orange
  '#8E44AD',  // Deep Purple
  '#2ECC71',  // Emerald
  '#E67E22'   // Carrot
];

const UserCube = ({ user, index }) => {
  const router = useRouter();
  const [imageError, setImageError] = useState(false);
  
  const [fontsLoaded] = useFonts({
    PokemonFont: require('../../../../assets/fonts/pokemon_Gen1.ttf'),
  });

  if (!fontsLoaded) return null;

  return (
    <TouchableOpacity
      style={[styles.cube, { backgroundColor: cubeColors[index % cubeColors.length] }]}
      onPress={() => router.push(`/stats/${user.name}`)}
    >
      <Image 
        source={getTrainerImage(user.name)}
        style={styles.sprite}
        resizeMode="contain"
        onError={() => setImageError(true)}
      />
      <View style={styles.infoContainer}>
        <Text style={styles.name}>{user.name}</Text>
        <Text style={styles.stats}>{user.statsCount || 0} stats</Text>
      </View>
    </TouchableOpacity>
  );
};

const styles = StyleSheet.create({
  cube: {
    width: 150,
    height: 150,
    margin: 10,
    padding: 10,
    borderRadius: 10,
    alignItems: 'center',
    justifyContent: 'center',
    borderWidth: 2,
    borderColor: 'black',
    shadowColor: '#000',
    shadowOffset: {
      width: 0,
      height: 2,
    },
    shadowOpacity: 0.25,
    shadowRadius: 3.84,
    elevation: 5,
  },
  sprite: {
    width: 80,
    height: 80,
    marginBottom: 5,
  },
  infoContainer: {
    alignItems: 'center',
  },
  name: {
    fontFamily: 'PokemonFont',
    fontSize: 16,
    color: 'black',
    textAlign: 'center',
  },
  stats: {
    fontFamily: 'PokemonFont',
    fontSize: 14,
    color: 'black',
    textAlign: 'center',
  }
});

export default UserCube;
