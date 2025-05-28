import React from 'react';
import ReactDOM from 'react-dom';

// Component chính
function App() {
  const [count, setCount] = React.useState(0);

  return (
    <div style={{ textAlign: 'center', marginTop: '50px' }}>
      <h1>Đếm: {count}</h1>
      <button onClick={() => setCount(count + 1)}>Tăng</button>
      <button onClick={() => setCount(count - 1)}>Giảm</button>
    </div>
  );
}

// Render ứng dụng
ReactDOM.render(<App />, document.getElementById('root'));