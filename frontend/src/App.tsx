import { useState } from 'react'

function App() {
  const [response, setResponse] = useState("No response yet.") ;

  async function testBackend() {
    try {
      const res = await fetch("http://127.0.0.1:8080/api/test");
      const data = await res.json();
      setResponse(JSON.stringify(data));
    } catch (error) {
      setResponse("Request failed.");
      console.error(error);
    }
  }

  return (
    <div style={{ padding: "2rem", fontFamily: "sans-serif" }}>
      <h1>Crewgle</h1>
      <p>{response}</p>
      <button onClick={testBackend}>Test Backend</button>
    </div>
  );
}

export default App;
