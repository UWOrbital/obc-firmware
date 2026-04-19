import InputGroup from "react-bootstrap/InputGroup";
import Form from "react-bootstrap/Form";
import Table from "react-bootstrap/Table";
import { useEffect, useRef, useState } from "react";

const MISSON_COMMAND_PREFIX = "MCC_";

function MissionCommands() {
  const [commands, setCommands] = useState("");
  const [commandResponse, setCommandResponse] = useState("");
  const [filterText, setFilterText] = useState("");
  const inputRef = useRef(null);

  useEffect(() => {
    try {
      const command = localStorage.getItem(MISSON_COMMAND_PREFIX + "command");
      const response = localStorage.getItem(MISSON_COMMAND_PREFIX + "response");
      if (!command) return;
      if (!response) return;
      setCommands(command);
      setCommandResponse(response);
    } catch (error) {
      console.error(error);
    }
  }, []);

  // const handleChange = (event) => {
  //   setCommands(event.target.value);
  //   localStorage.setItem(MISSON_COMMAND_PREFIX + "command", event.target.value);
  // }

  const handleCommand = () => {
    try {
      const requestOptions = {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ command: commands }),
      };
      fetch("http://localhost:5000/mission-control", requestOptions)
        .then((response) => response.json())
        .then((data) => {
          setCommandResponse(data.response);
          localStorage.setItem(
            MISSON_COMMAND_PREFIX + "response",
            data.response,
          );
        });
    } catch (error) {
      console.error(error);
    }
  };

  const clear = () => {
    try {
      localStorage.clear();
    } catch (error) {
      console.error(error);
    }
    setCommands("");
    setCommandResponse("");
    // inputRef.current.value = ''
  };

  return (
    <div className="missionCommands layout">
      <InputGroup size="sm" className="mb-3">
        <InputGroup.Text id="inputGroup-sizing-sm" onClick={handleCommand}>
          Enter Command
        </InputGroup.Text>
        <Form.Control
          aria-label="Small"
          aria-describedby="inputGroup-sizing-sm"
          // onChange={handleChange}
          ref={inputRef}
        />
      </InputGroup>
      <div style={{ marginBottom: "1rem" }}>
        <input
          placeholder="Filter output..."
          value={(commandResponse && commandResponse.toString().includes(filterText || "")) ? commandResponse : ""}
          onChange={(e) => setFilterText(e.target.value)}
          style={{ marginRight: "1rem" }}
        />
      </div>
      <Table striped bordered hover>
        <thead>
          <tr>
            <th>Output: {Date()}</th>
          </tr>
        </thead>
        <tbody>
          <tr>
            {commandResponse &&
              <td>
                {filterText ?
                  commandResponse.toString().includes(filterText) ?
                    commandResponse : "No matching output"
                  : commandResponse}
              </td>
            }
          </tr>
        </tbody>
      </Table>
      <button onClick={clear}>Clear</button>
    </div>
  );
}

export default MissionCommands;