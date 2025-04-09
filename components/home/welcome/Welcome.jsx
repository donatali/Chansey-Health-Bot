import { useState } from "react";
import { MaterialCommunityIcons } from '@expo/vector-icons';
import {
  View,
  Text,
  TextInput,
  TouchableOpacity,
  Image,
  FlatList,
  StyleSheet,
} from "react-native";
import { useRouter } from "expo-router";
import { useFonts } from "expo-font";
import { SIZES } from "../../../constants";
import mockData from "../../../constants/trainerData.js";
import allNames from "../../../constants/getAllNames";


const TrainerTypes = ["Student", "IA/GSI", "Professor/Mentor"];

const Welcome = ({ searchTerm, setSearchTerm, handleClick }) => {
  const router = useRouter();
  const [activeTrainerType, setActiveTrainerType] = useState("Student");

  return (
      <View style={styles.messageBox}>
        <Text style={styles.userName}>Find your sprite or type name !</Text>

      <View style={styles.searchContainer}>
        <View style={styles.searchWrapper}>
          <TextInput
            style={styles.searchInput}
            value={searchTerm}
            onChangeText={(text) => setSearchTerm(text)}
            placeholder='What is your name?'
          />
        </View>

        <TouchableOpacity style={styles.searchBtn} onPress={handleClick}>
          <MaterialCommunityIcons
            name="pokeball"
            size={50}
            color="red"
          />
        </TouchableOpacity>
      </View>

      <View style={styles.tabsContainer}>
        <FlatList
          data={TrainerTypes}
          renderItem={({ item }) => (
            <TouchableOpacity
              style={styles.tab(activeTrainerType, item)}
              onPress={() => {
                setActiveTrainerType(item);
                router.push(`/search/${encodeURIComponent(item)}`);
              }}
            >
              <Text style={styles.tabText(activeTrainerType, item)}>{item}</Text>
            </TouchableOpacity>
          )}
          keyExtractor={(item) => item}
          contentContainerStyle={{ columnGap: SIZES.medium }}
          horizontal
        />
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  messageBox: {
    backgroundColor: 'white',
    borderColor: 'black',
    borderWidth: 2,
    borderRadius: 10,
    padding: 16,
    marginBottom: 20,
  },
  userName: {
    fontFamily: 'PokemonFont',
    fontSize: 20,
    marginBottom: 5,
    color: 'black',
  },
  welcomeMessage: {
    fontFamily: 'PokemonFont',
    fontSize: 16,
    marginBottom: 15,
    color: 'black',
  },
  searchContainer: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 15,
  },
  searchWrapper: {
    flex: 1,
    backgroundColor: '#F3F4F8',
    borderRadius: 8,
    paddingHorizontal: 10,
    marginRight: 10,
  },
  searchInput: {
    height: 40,
    fontFamily: 'PokemonFont',
    fontSize: 14,
    color: 'black',
  },
  searchBtn: {
    padding: 4,
  },
  tabsContainer: {
    flexDirection: 'row',
  },
  tab: (activeType, item) => ({
    paddingVertical: 6,
    paddingHorizontal: 12,
    backgroundColor: activeType === item ? '#000' : '#eee',
    borderRadius: 8,
    borderWidth: 1,
    borderColor: '#000',
  }),
  tabText: (activeType, item) => ({
    fontFamily: 'PokemonFont',
    fontSize: 14,
    color: activeType === item ? 'white' : 'black',
  }),
});

export default Welcome;