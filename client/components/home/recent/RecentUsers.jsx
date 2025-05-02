import React, { useState, useCallback} from "react";
import { useRouter } from "expo-router";
import { View, Text, TouchableOpacity, ActivityIndicator, FlatList, StyleSheet } from "react-native";
import { useFonts } from "expo-font";
import { useFocusEffect } from '@react-navigation/native';

// Local styles defined below
import { COLORS, SIZES } from "../../../constants";
import UserCube from "../../common/cards/user/UserCube";
import useFetch from "../../../hook/useFetch";

// Memoized UserCube component for performance
const MemoizedUserCube = React.memo(UserCube);

// Memoized render item function to prevent unnecessary re-renders
const renderUser = ({ item, selectedUser, onPress, index }) => (
  <MemoizedUserCube
    user={item}
    selected={selectedUser}
    handlePress={onPress}
    index={index}
  />
);

const RecentUsers = () => {
  const router = useRouter();
  const [selectedUser, setSelectedUser] = useState(null);
  
  // Use the useFetch hook to get data from the recent users endpoint
  const { data: recentUsers, isLoading, error, refetch } = useFetch("top-users/recent", "GET");
  
  useFocusEffect(
    useCallback(() => {
      refetch();
    }, [])
  )
  // Load Pokemon font
  const [fontsLoaded] = useFonts({
    PokemonFont: require('../../../assets/fonts/pokemon_Gen1.ttf'),
  });
  
  if (!fontsLoaded) return null;

  // Memoized handler to prevent recreating function on each render
  const handleUserPress = useCallback((user) => {
    setSelectedUser(user.id);
    // Add a small delay for visual feedback before navigation
    setTimeout(() => {
      router.push(`/stats/${user.name}`);
    }, 150);
  }, [router]);
  
  // Memoized render function to create item components
  const renderItem = useCallback(({ item, index }) => {
    return renderUser({
      item,
      selectedUser,
      onPress: handleUserPress,
      index
    });
  }, [selectedUser, handleUserPress]);

  return (
    <View style={componentStyles.container}>
      <View style={componentStyles.headerBox}>
        <View style={componentStyles.header}>
          <Text style={componentStyles.headerTitle}>Most Recent Users</Text>
        </View>

        <View style={componentStyles.cardsContainer}>
          {isLoading ? (
            <ActivityIndicator size='large' color={COLORS.primary} />
          ) : error ? (
            <Text style={componentStyles.errorText}>Something went wrong</Text>
          ) : (
            <FlatList
              data={recentUsers}
              renderItem={renderItem}
              keyExtractor={(item) => item.id}
              contentContainerStyle={{ paddingHorizontal: SIZES.small }}
              horizontal
              showsHorizontalScrollIndicator={false}
              initialNumToRender={4}
              maxToRenderPerBatch={4}
              windowSize={3}
              removeClippedSubviews={true}
              getItemLayout={(data, index) => ({
                length: 180 + SIZES.small * 2, // Width of item + horizontal padding
                offset: (180 + SIZES.small * 2) * index,
                index,
              })}
            />
          )}
        </View>
      </View>
    </View>
  );
};

// Local styles that match the Welcome component
const componentStyles = StyleSheet.create({
  container: {
    marginTop: SIZES.medium,
    marginBottom: SIZES.medium,
  },
  headerBox: {
    backgroundColor: 'white',
    borderColor: 'black',
    borderWidth: 2,
    borderRadius: 10,
    padding: 16,
    marginBottom: 10,
  },
  header: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    marginBottom: 10,
  },
  headerTitle: {
    fontFamily: "PokemonFont",
    fontSize: 20,
    color: "black",
  },
  headerBtn: {
    fontFamily: "PokemonFont",
    fontSize: 14,
    color: COLORS.gray,
  },
  cardsContainer: {
    paddingVertical: SIZES.small / 2,
  },
  errorText: {
    fontFamily: "PokemonFont",
    color: COLORS.red,
    fontSize: 14,
  }
});

export default RecentUsers;

