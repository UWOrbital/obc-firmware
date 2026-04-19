import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import { faPlus } from '@fortawesome/free-solid-svg-icons'
import { mockCommandsList } from "../../../utils/mock-data.ts" // temporary commands list
import type { Command } from "../../../utils/types.ts"

import { Button } from "@/components/ui/button"
import {
  DropdownMenu,
  DropdownMenuCheckboxItem,
  DropdownMenuContent,
  DropdownMenuLabel,
  DropdownMenuSeparator,
  DropdownMenuTrigger,
} from "@/components/ui/dropdown-menu"

function SelectCommand({selectedCommand, setCommand}: {selectedCommand: string; setCommand: (cmd: string) => void;}) {
  return (
     <DropdownMenu>
      <DropdownMenuTrigger asChild>
        <Button variant="outline" className="fixed bottom-10 left-10 z-10 rounded-full w-15 h-15 flex items-center justify-center hover:border-ring hover:ring-ring/50 hover:ring-[2px]"><FontAwesomeIcon icon={faPlus} size="xl" /></Button>
      </DropdownMenuTrigger>
      <DropdownMenuContent className="w-56">
        <DropdownMenuLabel>Commands</DropdownMenuLabel>
        <DropdownMenuSeparator />
        {mockCommandsList.map((command: Command) => (
          <DropdownMenuCheckboxItem
            key={command.id}
            checked={selectedCommand === command.name}
            onCheckedChange={() => setCommand(command.name)}
          >
            {command.name}
          </DropdownMenuCheckboxItem>
        ))}
      </DropdownMenuContent>
    </DropdownMenu>
  )
}

export default SelectCommand
