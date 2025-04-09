import { useState, useEffect } from "react";
//import { io } from "socket.io-client";
import axios from "axios";
import AsyncStorage from "@react-native-async-storage/async-storage";


const BASE_URL = "http://localhost:3000";
const cacheKey = `cache_recieve`;
/*
const useLiveEvents = () => {
  const [events, setEvents] = useState([]);
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    const loadUserId = async () => {
      const id = await AsyncStorage.getItem('user_id');
      setUserId(id);
    };
    loadUserId();
  }, []);

  useEffect(() => {
    const fetchInitialData = async () => {
      try {
        const cached = await AsyncStorage.getItem(cacheKey);
        if (cached) {
          setEvents(JSON.parse(cached));
        }
        const res = await axios.get(`${BASE_URL}/receive`);
        if(res.data && res.data.events) {
          setEvents(res.data.events);
          await AsyncStorage.setItem(cacheKey, JSON.stringify(res.data.events));
        }
        setIsLoading(false);
      } catch (err) {
        console.error("Initial fetch failed:", err);
        setError(err);
        setIsLoading(false);
      }
    };
    fetchInitialData();
  }, []);

  useEffect(() => {
    const socket = io(BASE_URL);

    socket.on("connect", () => {
      console.log("WebSocket connected");
    });
    
    socket.on("new_event", async (newEvent) => {
      if(!userId || newEvent.user_id !== parseInt(userID)) {
        return;
      }
      
      setEvents((prev) => {
        
        const exists = prev.find(e =>
          e.timestamp === newEvent.timestamp &&
          e.pill_dispensed === newEvent.pill_dispensed
        );
        if (exists) return prev;
        
        const updated = [newEvent, ...prev];
        AsyncStorage.setItem(cacheKey, JSON.stringify(updated));
        return updated;
      });
    });

    socket.on("disconnect", () => {
      console.log("WebSocket disconnected");
    });

    return () => socket.disconnect();
  }, []);

  return { data: events, isLoading, error };
};
    
    
export default useLiveEvents;
*/


const useFetch = (endpoint, method = "GET", body = null) => {
  const [data, setData] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState(null);
  const [lastTimestamp, setLastTimestamp] = useState(null);
  const cacheKey = `cache_${endpoint}`;

  const fetchData = async () => {
    setIsLoading(true);

    try {
      const response = await axios({
        method,
        url: `${BASE_URL}/${endpoint}`,
        data: body,
      });

      //setData(response.data);
      //await AsyncStorage.setItem(cacheKey, JSON.stringify(response.data));
      //setError(null);
      const newData = response.data;
      if(newData && newData.length > 0) {
        const latestEvent = newData[newData.length - 1];
        if(latestEvent.timestamp !== lastTimestamp) {
          const updatedData = [...data, ...newData];
          setData(updatedData);
          setLastTimestamp(latestEvent.timestamp);
          await AsyncStorage.setItem(cacheKey, JSON.stringify(updatedData));
          setError(null);
        }
      }
    } catch (err) {
      console.warn(`Fetch error: ${err}. Trying cache....`);
      setError(err);
    
      try {
        const cacheData = await AsyncStorage.getItem(cacheKey);
        if (cacheData) {
          setData(JSON.parse(cacheData));
        }
      } catch (cacheErr) {
        console.error("Error landing cached data:", cacheErr);
      }
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => {
    let interval;

    const LoadInitialData = async () => {
      const cacheData = await AsyncStorage.getItem(cacheKey);
      if (cacheData) {
        setData(JSON.parse(cacheData));
      } else {
        fetchData();
      }
    };

    LoadInitialData();

    if(method == "GET") {
      //fetchData();

      if(!isLoading){
        fetchData();
      }

      interval = setInterval(() => {
        if(!isLoading) {
          fetchData();
      }
    }, 30000);
  }
    return () => clearInterval(interval);
  }, [endpoint, lastTimestamp, isLoading]);

  const refetch = () => {
    setIsLoading(true);
    fetchData();
  };

  return { data, isLoading, error, refetch };
};

export default useFetch;
